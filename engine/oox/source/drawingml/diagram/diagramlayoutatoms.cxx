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

#include "diagramlayoutatoms.hxx"

#include <array>
#include <cmath>
#include <editeng/editeng.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <functional>
#include <algorithm>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "layoutatomvisitorbase.hxx"

#include <basegfx/numeric/ftools.hxx>
#include <limits>
#include <sal/log.hxx>

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/properties.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>

using namespace ::com::sun::star;
using namespace ::cpo::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

namespace
{
/// Looks up the value of the rInternalName -> nProperty key in rProperties.
std::optional<sal_Int32> findProperty(const oox::drawingml::LayoutPropertyMap& rProperties,
                                      const OUString& rInternalName, sal_Int32 nProperty)
{
    std::optional<sal_Int32> oRet;

    auto it = rProperties.find(rInternalName);
    if (it != rProperties.end())
    {
        const oox::drawingml::LayoutProperty& rProperty = it->second;
        auto itProperty = rProperty.find(nProperty);
        if (itProperty != rProperty.end())
            oRet = itProperty->second;
    }

    return oRet;
}

/**
 * Determines if nUnit is a font unit (measured in points) or not (measured in
 * millimeters).
 */
bool isFontUnit(sal_Int32 nUnit)
{
    return nUnit == oox::XML_primFontSz || nUnit == oox::XML_secFontSz;
}

/// Determines which UNO property should be set for a given constraint type.
sal_Int32 getPropertyFromConstraint(sal_Int32 nConstraint)
{
    switch (nConstraint)
    {
        case oox::XML_lMarg:
            return oox::PROP_TextLeftDistance;
        case oox::XML_rMarg:
            return oox::PROP_TextRightDistance;
        case oox::XML_tMarg:
            return oox::PROP_TextUpperDistance;
        case oox::XML_bMarg:
            return oox::PROP_TextLowerDistance;
    }

    return 0;
}

// True for the names a layout gives to a value it works out for itself, userA to userZ. The
// tokens are not one unbroken run, userDrawn and userName sit among them, so they are named.
bool isUserVariable(sal_Int32 nType)
{
    switch (nType)
    {
        case oox::XML_userA:
        case oox::XML_userB:
        case oox::XML_userC:
        case oox::XML_userD:
        case oox::XML_userE:
        case oox::XML_userF:
        case oox::XML_userG:
        case oox::XML_userH:
        case oox::XML_userI:
        case oox::XML_userJ:
        case oox::XML_userK:
        case oox::XML_userL:
        case oox::XML_userM:
        case oox::XML_userN:
        case oox::XML_userO:
        case oox::XML_userP:
        case oox::XML_userQ:
        case oox::XML_userR:
        case oox::XML_userS:
        case oox::XML_userT:
        case oox::XML_userU:
        case oox::XML_userV:
        case oox::XML_userW:
        case oox::XML_userX:
        case oox::XML_userY:
        case oox::XML_userZ:
            return true;
        default:
            break;
    }

    return false;
}

/**
 * Determines if pShape is (or contains) a presentation of a data node of type
 * nType.
 */
bool containsDataNodeType(const oox::drawingml::ShapePtr& pShape, sal_Int32 nType)
{
    if (pShape->getDataNodeType() == nType)
        return true;

    for (const auto& pChild : pShape->getChildren())
    {
        if (containsDataNodeType(pChild, nType))
            return true;
    }

    return false;
}
}

namespace oox::drawingml {
// set the guide rName to nValue: an existing guide takes the new value in its place, a
// missing one is added at the end
static void setGuideValue(CustomShapeGuideContainer& rGuides, const OUString& rName,
                          sal_Int32 nValue)
{
    const OUString aValue(OUString::number(nValue));
    CustomShapeGuideContainer aGuides;
    bool bFound(false);
    for (const CustomShapeGuide& rGuide : rGuides)
    {
        if (rGuide.maName == rName)
        {
            aGuides.push_back({ rName, aValue });
            bFound = true;
        }
        else
            aGuides.push_back(rGuide);
    }
    if (!bFound)
        aGuides.push_back({ rName, aValue });
    rGuides = std::move(aGuides);
}

// The names of the layout nodes below every loop over the sibTrans points from rAtom down, the
// nodes that stand for a transition between two nodes of the data.
static void gatherTransitionNames(const LayoutAtom& rAtom, std::set<OUString>& rOut,
                                  bool bBelowTransitionLoop = false)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        bool bBelow(bBelowTransitionLoop);
        if (const ForEachAtom* pLoop = dynamic_cast<const ForEachAtom*>(pChild.get()))
            bBelow = bBelow || pLoop->iterator().mnPtType == XML_sibTrans;
        if (const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get()))
        {
            if (bBelow)
                rOut.insert(pNode->getName());
            continue;
        }
        gatherTransitionNames(*pChild, rOut, bBelow);
    }
}

namespace
{
void gatherDecidedConstraints(const SmartArtDiagram& rDgm, const LayoutAtom& rAtom,
                              const rtl::Reference<svx::diagram::Point>& rPoint,
                              std::vector<Constraint>& rOut);
}

static void gatherLayoutNodes(const LayoutAtom& rAtom,
                              std::map<OUString, const LayoutNode*>& rOut);

namespace
{
// what a composite draws, as parts of its width and height from its top left: the boxes of
// its drawn children from their "l", "r", "w", "t" and "h", for the branch rPoint decides;
// 0 and 1 where nothing says more
struct DrawnParts
{
    double fLeft = 0.0, fTop = 0.0, fRight = 1.0, fBottom = 1.0;
};

DrawnParts drawnPartsOf(const SmartArtDiagram& rDgm, const LayoutNode& rNode,
                        const rtl::Reference<svx::diagram::Point>& rPoint,
                        const ShapePtr& rComposite)
{
    DrawnParts aOut;
    std::vector<Constraint> aOwn;
    gatherDecidedConstraints(rDgm, rNode, rPoint, aOwn);
    std::map<OUString, double> aWidth, aHeight, aLeft, aRight, aTop;
    for (int nPass = 0; nPass < 3; ++nPass)
        for (const Constraint& rOne : aOwn)
        {
            if (rOne.mnFor != XML_ch || rOne.msForName.isEmpty()
                || (rOne.mnRefType != XML_w && rOne.mnRefType != XML_h))
                continue;
            const double fPart(rOne.mfFactor);
            std::optional<double> oOf;
            if (rOne.msRefForName.isEmpty())
                oOf = 1.0;
            else if (rOne.mnRefType == XML_w && aWidth.count(rOne.msRefForName))
                oOf = aWidth.at(rOne.msRefForName);
            else if (rOne.mnRefType == XML_h && aHeight.count(rOne.msRefForName))
                oOf = aHeight.at(rOne.msRefForName);
            if (!oOf)
                continue;
            // only a side against the same side counts: a width of the width, a top of the height
            const bool bAcross(rOne.mnRefType == XML_w);
            switch (rOne.mnType)
            {
                case XML_w:
                    if (bAcross)
                        aWidth[rOne.msForName] = fPart * *oOf;
                    break;
                case XML_l:
                    if (bAcross)
                        aLeft[rOne.msForName] = fPart * *oOf;
                    break;
                case XML_r:
                    if (bAcross)
                        aRight[rOne.msForName] = fPart * *oOf;
                    break;
                case XML_h:
                    if (!bAcross)
                        aHeight[rOne.msForName] = fPart * *oOf;
                    break;
                case XML_t:
                    if (!bAcross)
                        aTop[rOne.msForName] = fPart * *oOf;
                    break;
                default:
                    break;
            }
        }
    bool bAny(false);
    double fLeft(1.0), fRight(0.0), fTop(1.0), fBottom(0.0);
    for (const ShapePtr& pChild : rComposite->getChildren())
    {
        const OUString& rName(pChild->getInternalName());
        if (!aWidth.count(rName) || !aHeight.count(rName)
            || pChild->getCustomShapeProperties()->getShapePresetType() == 0)
            continue;
        const double fAt(aLeft.count(rName)    ? aLeft.at(rName)
                         : aRight.count(rName) ? aRight.at(rName) - aWidth.at(rName)
                                               : 0.0);
        const double fDown(aTop.count(rName) ? aTop.at(rName) : 0.0);
        fLeft = std::min(fLeft, fAt);
        fRight = std::max(fRight, fAt + aWidth.at(rName));
        fTop = std::min(fTop, fDown);
        fBottom = std::max(fBottom, fDown + aHeight.at(rName));
        bAny = true;
    }
    if (bAny)
        aOut = DrawnParts{ fLeft, fTop, fRight, fBottom };
    return aOut;
}
}

void SnakeAlg::layoutShapeChildren(const SmartArtDiagram& rDgm, const AlgAtom& rAlg,
                                   const ShapePtr& rShape,
                                   const std::vector<Constraint>& rConstraints)
{
    if (rShape->getChildren().empty() || rShape->getSize().Width == 0
        || rShape->getSize().Height == 0)
        return;

    // A space that stands for no point of the data and states its width by its name is the gap
    // between the cells, as a part of a cell's width: the calendars put a space of -0.01 of the
    // width after every day, the last one trailing, and the days lap over each other by that.
    // The spaces are taken out before the cells are laid out, the gap stays.
    std::optional<double> oGapOfCell;
    bool bGapTrails = false;
    // the transitions taken out as spaces, and the width one states as a part of a cell
    bool bTransitionsTakenOut = false;
    double fTransitionOfCell = 0.0;
    // how far down what a cell draws reaches, as a part of the cell's height, at least all of it
    double fContentDown = 1.0;
    // the side of the cell the transition's width is a part of, XML_w or XML_h
    sal_Int32 nTransitionOfSide = XML_w;
    OUString aTransitionName;
    {
        const auto aWidthFactorOf = [&rConstraints](const OUString& rName) {
            std::optional<double> oFactor;
            for (const Constraint& rConstraint : rConstraints)
                if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_w
                    && rConstraint.msForName == rName && rConstraint.msRefForName.isEmpty()
                    && rConstraint.mnRefFor != XML_ch)
                    oFactor = rConstraint.mfFactor;
            return oFactor;
        };
        std::optional<double> oNodeFactor;
        for (const ShapePtr& rChild : rShape->getChildren())
        {
            const bool bSpace(rChild->getServiceName() == "com.sun.star.drawing.GroupShape"
                              && rChild->getChildren().empty());
            const std::optional<double> oFactor(aWidthFactorOf(rChild->getInternalName()));
            if (bSpace && oFactor)
            {
                oGapOfCell = *oFactor;
                bGapTrails = rChild == rShape->getChildren().back();
            }
            else if (!bSpace && !oNodeFactor)
                oNodeFactor = oFactor;
        }
        if (oGapOfCell)
            *oGapOfCell /= oNodeFactor.value_or(1.0);
        // A transition that is an empty group is taken out like a space, and where the layout
        // states a width for it, "w for ch forName sibTrans refType w refFor ch refForName
        // composite fact 0.0001" on Picture_Accent_Blocks, it is the gap along the row, as wide
        // as that: the cells touch along the row there and the sp is the gap between the rows
        // only. A transition that states nothing takes no room and the sp stays along the row,
        // as in the picture lists whose drawings keep it.
        for (const ShapePtr& rChild : rShape->getChildren())
        {
            if (rChild->getServiceName() != "com.sun.star.drawing.GroupShape"
                || !rChild->getChildren().empty() || rChild->getDataNodeType() != XML_sibTrans)
                continue;
            const auto aOfTransition = [&rChild](const Constraint& rConstraint) {
                return rConstraint.mnFor == XML_ch && rConstraint.mfFactor > 0.0
                       && (rConstraint.msForName == rChild->getInternalName()
                           || (rConstraint.msForName.isEmpty()
                               && rConstraint.mnPointType == XML_sibTrans));
            };
            for (const Constraint& rConstraint : rConstraints)
                if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_w
                    && aOfTransition(rConstraint))
                {
                    bTransitionsTakenOut = true;
                    fTransitionOfCell = rConstraint.mfFactor;
                    nTransitionOfSide = XML_w;
                    aTransitionName = rChild->getInternalName();
                }
            // A transition as wide as it is high, "w for ch forName sibTrans refType h refFor ch
            // refForName sibTrans" on Picture_Strips, is that part of its height wide, and its
            // height a part of a side of the cell, 0.1 of the composite's height there.
            if (bTransitionsTakenOut)
                continue;
            for (const Constraint& rWidth : rConstraints)
            {
                if (rWidth.mnType != XML_w || rWidth.mnRefType != XML_h
                    || rWidth.msRefForName != rChild->getInternalName() || !aOfTransition(rWidth))
                    continue;
                for (const Constraint& rHeight : rConstraints)
                    if (rHeight.mnType == XML_h
                        && (rHeight.mnRefType == XML_w || rHeight.mnRefType == XML_h)
                        && !rHeight.msRefForName.isEmpty()
                        && rHeight.msRefForName != rChild->getInternalName()
                        && aOfTransition(rHeight))
                    {
                        bTransitionsTakenOut = true;
                        fTransitionOfCell = rWidth.mfFactor * rHeight.mfFactor;
                        nTransitionOfSide = rHeight.mnRefType;
                        aTransitionName = rChild->getInternalName();
                    }
            }
        }
        std::erase_if(rShape->getChildren(), [](const ShapePtr& aChild) {
            return aChild->getServiceName() == "com.sun.star.drawing.GroupShape"
                   && aChild->getChildren().empty();
        });
        if (rShape->getChildren().empty())
            return;
    }

    if (layoutBendingProcess(rAlg, rShape, rConstraints))
        return;

    // Parse constraints.
    double fChildAspectRatio = rShape->getChildren()[0]->getAspectRatio();

    // A composite cell whose children are stated as parts of its width, a picture as high as
    // wide and a caption of 0.15 of the width above it at 0.15 of the height, is as high as that
    // content makes it, and not what its ar says: Picture_Grid's cells are 1.176 of their width
    // high, the picture over 1 less the 0.15 of the height above it, where the ar of 0.7568
    // would make them 1.32. A child whose height is a part of the cell's height says nothing
    // about that, and the cell keeps its ar then.
    {
        const OUString& rCellName(rShape->getChildren()[0]->getInternalName());
        const LayoutNode* pCell(nullptr);
        std::function<void(const LayoutAtom&)> aFindCell = [&](const LayoutAtom& rAtom) {
            for (const LayoutAtomPtr& pChild : rAtom.getChildren())
            {
                if (pCell)
                    return;
                if (const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get()))
                    if (pNode->getName() == rCellName)
                    {
                        pCell = pNode;
                        return;
                    }
                aFindCell(*pChild);
            }
        };
        aFindCell(rAlg.getLayoutNode());
        std::map<OUString, std::pair<double, double>> aTopOf; // t as a part of w, of h
        std::map<OUString, std::pair<double, double>> aHeightOf; // h as a part of w, of h
        std::function<void(const LayoutAtom&)> aReadCell = [&](const LayoutAtom& rAtom) {
            for (const LayoutAtomPtr& pChild : rAtom.getChildren())
            {
                if (dynamic_cast<const LayoutNode*>(pChild.get()))
                    continue;
                const ConstraintAtom* pConstraint
                    = dynamic_cast<const ConstraintAtom*>(pChild.get());
                if (!pConstraint)
                {
                    aReadCell(*pChild);
                    continue;
                }
                const Constraint& rConstraint(pConstraint->getConstraint());
                if (rConstraint.mnFor != XML_ch || rConstraint.msForName.isEmpty()
                    || !rConstraint.msRefForName.isEmpty()
                    || (rConstraint.mnOperator != XML_none && rConstraint.mnOperator != XML_equ))
                    continue;
                // a fact of 0 is a stated 0, the caption at the very top; the parser gives 1
                // where the file states none
                const double fFactor(rConstraint.mfFactor);
                if (rConstraint.mnType == XML_t && rConstraint.mnRefType == XML_w)
                    aTopOf[rConstraint.msForName].first = fFactor;
                else if (rConstraint.mnType == XML_t && rConstraint.mnRefType == XML_h)
                    aTopOf[rConstraint.msForName].second = fFactor;
                else if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w)
                    aHeightOf[rConstraint.msForName].first = fFactor;
                else if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_h)
                    aHeightOf[rConstraint.msForName].second = fFactor;
            }
        };
        if (pCell)
            aReadCell(*pCell);
        std::optional<double> oHeightOfWidth;
        bool bContentSaysAll(pCell != nullptr && !aHeightOf.empty());
        for (const auto& rEntry : aHeightOf)
        {
            const auto aTop = aTopOf.find(rEntry.first);
            const double fTopOfWidth(aTop != aTopOf.end() ? aTop->second.first : 0.0);
            const double fTopOfHeight(aTop != aTopOf.end() ? aTop->second.second : 0.0);
            if (rEntry.second.second > 0.0 || rEntry.second.first <= 0.0 || fTopOfHeight >= 1.0)
            {
                bContentSaysAll = false;
                break;
            }
            // the child's bottom is the cell's height: h = (tw + hw) w + th h
            const double fOfWidth((fTopOfWidth + rEntry.second.first) / (1.0 - fTopOfHeight));
            oHeightOfWidth = std::max(oHeightOfWidth.value_or(0.0), fOfWidth);
        }
        if (bContentSaysAll && oHeightOfWidth && *oHeightOfWidth > 0.0)
            fChildAspectRatio = 1.0 / *oHeightOfWidth;
        // The snake may state the cell's height as a part of the cell's own width, "h for ch
        // forName node refType w refFor ch refForName node fact 0.6" on Basic_Block_List, and
        // that is its proportions where the cell says nothing itself.
        if (fChildAspectRatio == 0.0)
            for (const Constraint& rConstraint : rConstraints)
                if (rConstraint.mnFor == XML_ch && rConstraint.msForName == rCellName
                    && rConstraint.msRefForName == rCellName && rConstraint.mnType == XML_h
                    && rConstraint.mnRefType == XML_w && rConstraint.mfFactor > 0.0
                    && rConstraint.mnOperator == XML_none)
                    fChildAspectRatio = 1.0 / rConstraint.mfFactor;

        // The transition is a part of the cell as the snake states it, "w for ch forName
        // composite refType w" and "h ... refType h", the box the composite then shrinks to its
        // ar in. Captioned_Pictures's composites of ar 0.85 stand in a box as wide to its height
        // as the room, 1.5, and their gap along the row is 0.1 of that box's width, 0.15 of
        // their height; the gap is kept as a part of the cell's own width here.
        std::optional<double> oStatedWidth;
        std::optional<double> oStatedHeight;
        for (const Constraint& rConstraint : rConstraints)
        {
            if (rConstraint.mnFor != XML_ch || rConstraint.msForName != rCellName
                || !rConstraint.msRefForName.isEmpty() || rConstraint.mnRefFor == XML_ch
                || rConstraint.mfFactor <= 0.0 || rConstraint.mfValue != 0.0)
                continue;
            const double fOf(rConstraint.mnRefType == XML_w   ? rShape->getSize().Width
                             : rConstraint.mnRefType == XML_h ? rShape->getSize().Height
                                                              : 0.0);
            if (fOf <= 0.0)
                continue;
            if (rConstraint.mnType == XML_w)
                oStatedWidth = fOf * rConstraint.mfFactor;
            else if (rConstraint.mnType == XML_h)
                oStatedHeight = fOf * rConstraint.mfFactor;
        }
        if (bTransitionsTakenOut && fChildAspectRatio > 0.0)
        {
            // the stated box's width and height, each as a part of the cell's width
            double fBoxWidth(1.0);
            double fBoxHeight(1.0 / fChildAspectRatio);
            if (oStatedWidth && oStatedHeight)
            {
                const double fStatedRatio(*oStatedWidth / *oStatedHeight);
                if (fChildAspectRatio < fStatedRatio)
                    fBoxWidth = fStatedRatio / fChildAspectRatio;
                else
                    fBoxHeight = 1.0 / fStatedRatio;
            }
            fTransitionOfCell *= nTransitionOfSide == XML_h ? fBoxHeight : fBoxWidth;
        }

        // What a cell draws can reach below the cell, and a row is as high as that: Picture_
        // Strips's rect1 starts at 0.13 of the composite's height and is 0.9 of it high, and the
        // drawing has its rows 1.13 of the height apart, the 1.03 down to rect1's bottom and the
        // sp after it. The cell's constraints are read the way the layout decided them for the
        // first cell.
        if (pCell && fChildAspectRatio > 0.0)
        {
            rtl::Reference<svx::diagram::Point> xCellPoint;
            for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                if (rEntry.second == rShape->getChildren()[0])
                {
                    xCellPoint = rEntry.first;
                    break;
                }
            std::vector<Constraint> aCellConstraints;
            gatherDecidedConstraints(rDgm, *pCell, xCellPoint, aCellConstraints);
            // t and h of every child as parts of the cell's height
            std::map<OUString, std::pair<std::optional<double>, std::optional<double>>> aDown;
            for (const Constraint& rConstraint : aCellConstraints)
            {
                if (rConstraint.mnFor != XML_ch || rConstraint.msForName.isEmpty()
                    || !rConstraint.msRefForName.isEmpty() || rConstraint.mfValue != 0.0
                    || (rConstraint.mnOperator != XML_none && rConstraint.mnOperator != XML_equ)
                    || (rConstraint.mnRefType != XML_w && rConstraint.mnRefType != XML_h))
                    continue;
                const double fOfHeight(rConstraint.mfFactor
                                       * (rConstraint.mnRefType == XML_w ? fChildAspectRatio
                                                                         : 1.0));
                if (rConstraint.mnType == XML_t)
                    aDown[rConstraint.msForName].first = fOfHeight;
                else if (rConstraint.mnType == XML_h)
                    aDown[rConstraint.msForName].second = fOfHeight;
            }
            for (const auto& rEntry : aDown)
                if (rEntry.second.first && rEntry.second.second)
                    fContentDown
                        = std::max(fContentDown, *rEntry.second.first + *rEntry.second.second);
        }
    }
    double fShapeHeight = rShape->getSize().Height;
    double fShapeWidth = rShape->getSize().Width;
    // Check if we have a child aspect ratio. If so, need to shrink one dimension to
    // achieve that ratio.
    if (fChildAspectRatio && fShapeHeight && fChildAspectRatio < (fShapeWidth / fShapeHeight))
    {
        fShapeWidth = fShapeHeight * fChildAspectRatio;
    }

    double fSpaceFromConstraint = 1.0;
    bool bStatesASpace = false;
    // An sp stated as a part of a width, "sp refType w refFor ch refForName composite fact
    // 0.113" on Picture_Accent_Blocks, is that length both ways, so the gap between the rows is
    // that part of the cell's width and not of its height: 851 there for a cell 7523 wide and
    // 6269 high, where 0.113 of the height would be 708.
    bool bSpaceOfWidth = false;
    LayoutPropertyMap aPropertiesByName;
    std::map<sal_Int32, LayoutProperty> aPropertiesByType;
    LayoutProperty& rParent = aPropertiesByName[u""_ustr];
    rParent[XML_w] = fShapeWidth;
    rParent[XML_h] = fShapeHeight;
    for (const auto& rConstr : rConstraints)
    {
        if (rConstr.mnRefType == XML_w || rConstr.mnRefType == XML_h)
        {
            if (rConstr.mnType == XML_sp && rConstr.msForName.isEmpty())
            {
                fSpaceFromConstraint = rConstr.mfFactor;
                bStatesASpace = true;
                bSpaceOfWidth = rConstr.mnRefType == XML_w;
                // An sp of the width of the transition taken out, "sp refType w refFor ch
                // refForName sibTrans" on Basic_Block_List, is that transition's part of the
                // cell, 0.1, both ways.
                if (rConstr.mnRefType == XML_w && bTransitionsTakenOut
                    && rConstr.msRefForName == aTransitionName)
                    fSpaceFromConstraint = rConstr.mfFactor * fTransitionOfCell;
            }
        }

        // The entry under the empty name is the parent's own size, which the named entries refer
        // to. A constraint that names no shape and no point type is read further down, not into
        // that entry; one that names a point type goes into the entries by type as before.
        if (rConstr.msForName.isEmpty() && rConstr.mnPointType == XML_all)
            continue;

        auto itRefForName = aPropertiesByName.find(rConstr.msRefForName);
        if (itRefForName == aPropertiesByName.end())
        {
            continue;
        }

        auto it = itRefForName->second.find(rConstr.mnRefType);
        if (it == itRefForName->second.end())
        {
            continue;
        }

        if (rConstr.mfValue != 0.0)
        {
            continue;
        }

        sal_Int32 nValue = it->second * rConstr.mfFactor;

        if (rConstr.mnPointType == XML_all)
        {
            aPropertiesByName[rConstr.msForName][rConstr.mnType] = nValue;
        }
        else
        {
            aPropertiesByType[rConstr.mnPointType][rConstr.mnType] = nValue;
        }
    }

    std::vector<sal_Int32> aShapeWidths(rShape->getChildren().size());
    for (size_t i = 0; i < rShape->getChildren().size(); ++i)
    {
        ShapePtr pChild = rShape->getChildren()[i];
        if (!pChild->getDataNodeType())
        {
            // TODO handle the case when the requirement applies by name, not by point type.
            aShapeWidths[i] = fShapeWidth;
            continue;
        }

        auto itNodeType = aPropertiesByType.find(pChild->getDataNodeType());
        if (itNodeType == aPropertiesByType.end())
        {
            aShapeWidths[i] = fShapeWidth;
            continue;
        }

        auto it = itNodeType->second.find(XML_w);
        if (it == itNodeType->second.end())
        {
            aShapeWidths[i] = fShapeWidth;
            continue;
        }

        aShapeWidths[i] = it->second;
    }

    bool bSpaceFromConstraints = fSpaceFromConstraint != 1.0;

    const AlgAtom::ParamMap& rMap = rAlg.getMap();
    const sal_Int32 nDir = rMap.count(XML_grDir) ? rMap.find(XML_grDir)->second : XML_tL;
    sal_Int32 nIncX = 1;
    sal_Int32 nIncY = 1;
    switch (nDir)
    {
        case XML_tL:
            nIncX = 1;
            nIncY = 1;
            break;
        case XML_tR:
            nIncX = -1;
            nIncY = 1;
            break;
        case XML_bL:
            nIncX = 1;
            nIncY = -1;
            break;
        case XML_bR:
            nIncX = -1;
            nIncY = -1;
            break;
    }

    sal_Int32 nCount = rShape->getChildren().size();
    // The gap between the cells is what a space child or a sp constraint states, and nothing when
    // the layout states neither: the cells of such a row touch, as the days of a calendar do. A
    // sp constraint of the whole of another shape's width is read as its factor 1.0 here, which
    // is no gap to use, and the row keeps the 0.3 it had for that.
    double fSpace = oGapOfCell            ? *oGapOfCell
                    : bSpaceFromConstraints ? fSpaceFromConstraint
                    : bStatesASpace         ? 0.3
                                            : 0.0;
    // the gap between two rows as a part of a cell's height
    double fRowSpace(bSpaceOfWidth && bSpaceFromConstraints && fChildAspectRatio > 0.0
                         ? fSpace * fChildAspectRatio
                         : fSpace);
    const bool bTransitionsAmongChildren(std::any_of(
        rShape->getChildren().begin(), rShape->getChildren().end(),
        [](const ShapePtr& rChild) { return rChild->getDataNodeType() == XML_sibTrans; }));
    // With the transitions among the children a row's gaps are those transitions, and sp is the
    // gap between the rows only: Picture_Caption_List's pictures stand their sibTrans of 0.1
    // apart along the row and its sp of 0.1 between the rows, not both along the row. The
    // transitions are children and take their place in the flow themselves, so the placing adds
    // nothing between two children; the fit of the grid counts a transition's width, stated for
    // every sibTrans as a part of a node, between two nodes.
    // the gap along the row is the transition's stated width where the transitions were taken
    // out, nothing where they stand among the children and take their room themselves
    double fAlongSpace(bTransitionsTakenOut        ? fTransitionOfCell
                       : bTransitionsAmongChildren ? 0.0
                                                   : fSpace);
    double fAlongFit(bTransitionsTakenOut ? fTransitionOfCell : fSpace);
    if (bTransitionsAmongChildren)
    {
        fAlongFit = 0.0;
        for (const Constraint& rConstraint : rConstraints)
            if (rConstraint.mnType == XML_w && rConstraint.mnFor == XML_ch
                && rConstraint.msForName.isEmpty() && rConstraint.mnPointType == XML_sibTrans
                && rConstraint.mnRefType == XML_w && rConstraint.mfFactor > 0.0)
                fAlongFit = rConstraint.mfFactor;
    }
    double fAspectRatio = 0.54; // diagram should not spill outside, earlier it was 0.6

    // A layout can say itself where the flow breaks into the next line instead of leaving
    // that to the fit. With bkpt fixed the file names how many shapes go into one line and
    // bkPtFixedVal carries that number, and flowDir says whether a line is a row or a column.
    const sal_Int32 nBreak = rMap.count(XML_bkpt) ? rMap.find(XML_bkpt)->second : XML_endCnv;
    const sal_Int32 nBreakAt
        = rMap.count(XML_bkPtFixedVal) ? rMap.find(XML_bkPtFixedVal)->second : 2;
    const sal_Int32 nFlowDir = rMap.count(XML_flowDir) ? rMap.find(XML_flowDir)->second : XML_row;

    sal_Int32 nCol = 1;
    sal_Int32 nRow = 1;
    sal_Int32 nMaxRowWidth = 0;
    if (nBreak == XML_fixed && nBreakAt >= 1)
    {
        if (nFlowDir == XML_col)
        {
            nRow = std::min(nCount, nBreakAt);
            nCol = std::ceil(static_cast<double>(nCount) / nRow);
        }
        else
        {
            nCol = std::min(nCount, nBreakAt);
            nRow = std::ceil(static_cast<double>(nCount) / nCol);
        }

        for (sal_Int32 i = 0; i < nCol && i < nCount; ++i)
            nMaxRowWidth += aShapeWidths[i];
    }
    else if (nCount <= fChildAspectRatio)
        // Child aspect ratio request (width/height) is N, and we have at most N shapes.
        // This means we don't need multiple columns.
        nRow = nCount;
    else if (fChildAspectRatio > 0.0)
    {
        // The children ask for a shape of their own, width to height, so for every count of
        // columns the grid, cells and gaps, has a size it can have in the room: the room's width
        // over the grid's, or its height over the grid's, whichever is less. The grid whose cells
        // come out largest is the one, and where two come out the same the one with more
        // columns: four nodes 0.85 wide to their height in a room 1.5 wide to its height stand
        // three and one, four of 1.1667 stand two and two. The grid is counted in nodes; where
        // the transitions stand among them as children of their own, a row of k nodes holds the
        // k transitions that follow them as well, and the row widths below count them in.
        const sal_Int32 nNodes(bTransitionsAmongChildren
                                   ? static_cast<sal_Int32>(std::count_if(
                                         rShape->getChildren().begin(),
                                         rShape->getChildren().end(),
                                         [](const ShapePtr& rChild) {
                                             return rChild->getDataNodeType() != XML_sibTrans;
                                         }))
                                   : nCount);
        double fBest(0.0);
        sal_Int32 nColumnsOfNodes(1);
        for (sal_Int32 nTry = 1; nTry <= nNodes; ++nTry)
        {
            const sal_Int32 nRows((nNodes + nTry - 1) / nTry);
            const double fAcross(nTry + (nTry - 1) * fAlongFit);
            const double fDown((nRows * fContentDown + (nRows - 1) * fRowSpace)
                                / fChildAspectRatio);
            const double fUnit(std::min(rShape->getSize().Width / fAcross,
                                        rShape->getSize().Height / fDown));
            if (fUnit >= fBest * (1.0 - 1e-9))
            {
                fBest = fUnit;
                nColumnsOfNodes = nTry;
            }
        }
        nCol = std::min(nCount, bTransitionsAmongChildren ? 2 * nColumnsOfNodes : nColumnsOfNodes);
        nRow = (nCount + nCol - 1) / nCol;
        for (sal_Int32 i = 0; i < nCol && i < nCount; ++i)
            nMaxRowWidth += aShapeWidths[i];
    }
    else
    {
        for (; nRow < nCount; nRow++)
        {
            nCol = std::ceil(static_cast<double>(nCount) / nRow);
            sal_Int32 nRowWidth = 0;
            for (sal_Int32 i = 0; i < nCol; ++i)
            {
                if (i >= nCount)
                {
                    break;
                }

                nRowWidth += aShapeWidths[i];
            }
            double fTotalShapesHeight = fShapeHeight * nRow;
            if (nRowWidth && fTotalShapesHeight / nRowWidth >= fAspectRatio)
            {
                if (nRowWidth > nMaxRowWidth)
                {
                    nMaxRowWidth = nRowWidth;
                }
                break;
            }
        }
    }

    // A width stated for every child at once, "w for ch refType=w fact=0.19" with no name and no
    // point type, is the width of a cell, and a row takes as many cells as fit into it; a height
    // stated the same way is the height of a cell. The dots of List_Dots stand in one row so.
    // A height stated as a part of the child's own width, "h for ch refType w refFor ch fact
    // 0.9" on Simple_Calendar, is that part of the cell's width, whatever the cell comes to.
    std::optional<sal_Int32> oCellWidth;
    std::optional<sal_Int32> oCellHeight;
    std::optional<double> oCellHeightOfWidth;
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnFor != XML_ch || !rConstraint.msForName.isEmpty()
            || rConstraint.mnPointType != XML_all || !rConstraint.msRefForName.isEmpty()
            || rConstraint.mfValue != 0.0)
            continue;
        if (rConstraint.mnRefFor == XML_ch)
        {
            if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w
                && rConstraint.mfFactor > 0.0)
                oCellHeightOfWidth = rConstraint.mfFactor;
            continue;
        }
        const sal_Int32 nOf(rConstraint.mnRefType == XML_w   ? rShape->getSize().Width
                            : rConstraint.mnRefType == XML_h ? rShape->getSize().Height
                                                             : 0);
        if (nOf <= 0)
            continue;
        if (rConstraint.mnType == XML_w)
            oCellWidth = nOf * rConstraint.mfFactor;
        else if (rConstraint.mnType == XML_h)
            oCellHeight = nOf * rConstraint.mfFactor;
    }
    if (oCellWidth && *oCellWidth <= 0)
        oCellWidth.reset();
    if (oCellWidth && nBreak != XML_fixed)
    {
        nCol = std::clamp<sal_Int32>((rShape->getSize().Width + fAlongSpace * *oCellWidth)
                                         / (*oCellWidth * (1.0 + fAlongSpace)),
                                     1, nCount);
        nRow = std::ceil(static_cast<double>(nCount) / nCol);
    }

    SAL_INFO("oox.drawingml", "Snake layout grid: " << nCol << "x" << nRow);

    // The gaps of a row: one less than the cells, or as many when a space trails the last cell.
    const sal_Int32 nGaps = bGapTrails ? nCol : nCol - 1;
    sal_Int32 nWidth = oCellWidth ? *oCellWidth : rShape->getSize().Width / (nCol + nGaps * fAlongSpace);
    // A row of stated cells wider than the snake shares the width out: Simple_Calendar states
    // every day as wide as the whole and stands its four days in one row of a fixed seven.
    if (oCellWidth && nCol > 0
        && static_cast<double>(nWidth) * (nCol + nGaps * fAlongSpace) > rShape->getSize().Width)
        nWidth = rShape->getSize().Width / (nCol + nGaps * fAlongSpace);
    awt::Size aChildSize(nWidth, oCellHeight ? *oCellHeight
                                 : oCellHeightOfWidth
                                     ? static_cast<sal_Int32>(nWidth * *oCellHeightOfWidth)
                                     : static_cast<sal_Int32>(nWidth * fAspectRatio));

    // The grid has to fit the room's height as well as its width. A cell whose height is a part
    // of its width shrinks, width and height together, until the rows with their gaps stand in
    // the height: Simple_3days of issue 16249 has three cells as wide as the whole and 0.9 of
    // that high in one row of a fixed seven, in a room of 11521440 by 2160000, and the drawing
    // has them 2399362 wide, the height over 0.9, the row in the middle of the width.
    if (oCellHeightOfWidth && !oCellHeight && nRow >= 1)
    {
        const double fGridHeight(aChildSize.Height
                                 * (nRow * fContentDown + (nRow - 1) * fRowSpace));
        if (fGridHeight > rShape->getSize().Height && fGridHeight > 0.0)
        {
            const double fShrink(rShape->getSize().Height / fGridHeight);
            nWidth = static_cast<sal_Int32>(nWidth * fShrink);
            aChildSize = awt::Size(nWidth, static_cast<sal_Int32>(nWidth * *oCellHeightOfWidth));
        }
    }

    // A snake in the offset mode, "off val=off", steps every row aside by a part of a cell, a
    // staircase: the second row stands alignOff of a cell further along than the first, the
    // third twice that. alignOff is stated on the snake itself, a bare value that is the part.
    const bool bOffsetMode((rMap.count(XML_off) ? rMap.find(XML_off)->second : XML_off)
                           == XML_off);
    // alignOff is 0 where the layout states none; every snake of the corpus in the offset mode
    // states it, Picture_Accent_Blocks and Step_Up_Process a whole cell, Step_Down_Process 0.48.
    double fAlignOff(0.0);
    if (bOffsetMode)
    {
        for (const Constraint& rConstraint : rConstraints)
            if (rConstraint.mnType == XML_alignOff && rConstraint.mfValue != 0.0)
                fAlignOff = rConstraint.mfValue;
    }

    // A staircase: one cell per row, the rows stepping aside by a whole cell, and a transition
    // between every two steps that is a cell of the flow like the others. Step_Up_Process has
    // its steps as wide as the whole and its transitions 0.103 of it, the rows lapping over each
    // other by 0.765 of a step's height. The transition is no row of its own then: a step stands
    // one step and one transition further along than the one before, the whole flight as wide
    // as the snake, and no higher than it.
    if (bOffsetMode && nCol == 1 && nRow > 1 && fChildAspectRatio > 0.0)
    {
        // the transitions are the layout nodes a loop over the sibTrans points builds
        std::set<OUString> aTransitionNames;
        gatherTransitionNames(rAlg.getLayoutNode(), aTransitionNames);
        std::vector<ShapePtr> aSteps;
        std::vector<ShapePtr> aBetween;
        for (const ShapePtr& rChild : rShape->getChildren())
            (aTransitionNames.count(rChild->getInternalName()) ? aBetween : aSteps)
                .push_back(rChild);
        if (aSteps.size() >= 2)
        {
            double fStepOfWidth(1.0);
            double fBetweenOfWidth(0.0);
            for (const Constraint& rConstraint : rConstraints)
            {
                if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
                    || !rConstraint.msRefForName.isEmpty() || rConstraint.mnRefFor == XML_ch
                    || rConstraint.mfFactor <= 0.0)
                    continue;
                if (rConstraint.msForName == aSteps.front()->getInternalName())
                    fStepOfWidth = rConstraint.mfFactor;
                else if (rConstraint.msForName == aBetween.front()->getInternalName())
                    fBetweenOfWidth = rConstraint.mfFactor;
            }
            // Without transitions among the children, Step_Down_Process's are empty groups
            // taken out above, a step stands alignOff of a step further along than the one
            // below, 0.48 of it there.
            const sal_Int32 nSteps(static_cast<sal_Int32>(aSteps.size()));
            const double fAdvance(aBetween.empty() ? fStepOfWidth * (fAlignOff - 1.0)
                                                   : fBetweenOfWidth);
            const double fFlight(nSteps * fStepOfWidth + (nSteps - 1) * fAdvance);
            double fStepWidth(rShape->getSize().Width * fStepOfWidth / fFlight);
            double fBetweenWidth(rShape->getSize().Width * fAdvance / fFlight);
            double fStepHeight(fStepWidth / fChildAspectRatio);
            double fPitch(1.0 + fSpace);
            double fHeight(fStepHeight * (1.0 + (nSteps - 1) * fPitch));
            if (fHeight > rShape->getSize().Height)
            {
                const double fShrink(rShape->getSize().Height / fHeight);
                fStepWidth *= fShrink;
                fBetweenWidth *= fShrink;
                fStepHeight *= fShrink;
                fHeight = rShape->getSize().Height;
            }
            double fTop((rShape->getSize().Height - fHeight) / 2.0);
            const double fFlightWidth(nSteps * fStepWidth + (nSteps - 1) * fBetweenWidth);
            double fLeft((rShape->getSize().Width - fFlightWidth) / 2.0);
            // without transitions the steps stand by what they draw: alignOff of the drawn width
            // along, (1 + sp) of the height over the square root of the ar down. Why the square
            // root I do not know, but it matches. The whole is fitted to the room and centred
            if (aBetween.empty())
            {
                std::map<OUString, const LayoutNode*> aStepNodes;
                gatherLayoutNodes(rAlg.getLayoutNode(), aStepNodes);
                const auto aPartsOf = [&](const ShapePtr& rStep) {
                    const auto aNode = aStepNodes.find(rStep->getInternalName());
                    rtl::Reference<svx::diagram::Point> xPoint;
                    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                        if (rEntry.second == rStep)
                        {
                            xPoint = rEntry.first;
                            break;
                        }
                    return aNode != aStepNodes.end() && aNode->second
                               ? drawnPartsOf(rDgm, *aNode->second, xPoint, rStep)
                               : DrawnParts();
                };
                const DrawnParts aFirst(aPartsOf(aSteps.front()));
                const DrawnParts aLast(aPartsOf(aSteps.back()));
                const double fAside(fAlignOff * (aFirst.fRight - aFirst.fLeft));
                const double fDown((1.0 + fSpace) / std::sqrt(fChildAspectRatio));
                const double fDrawnWide((nSteps - 1) * fAside + aLast.fRight - aFirst.fLeft);
                const double fDrawnHigh((nSteps - 1) * fDown + aLast.fBottom - aFirst.fTop);
                if (fDrawnWide > 0.0 && fDrawnHigh > 0.0)
                {
                    fStepWidth = std::min(rShape->getSize().Width / fDrawnWide,
                                          rShape->getSize().Height * fChildAspectRatio
                                              / fDrawnHigh);
                    fStepHeight = fStepWidth / fChildAspectRatio;
                    fBetweenWidth = (fAside - 1.0) * fStepWidth;
                    fPitch = fDown;
                    fLeft = (rShape->getSize().Width - fDrawnWide * fStepWidth) / 2.0
                            - aFirst.fLeft * fStepWidth;
                    fTop = (rShape->getSize().Height - fDrawnHigh * fStepHeight) / 2.0
                           - aFirst.fTop * fStepHeight;
                    fHeight = ((nSteps - 1) * fDown + 1.0) * fStepHeight;
                }
            }
            const awt::Size aStepSize(static_cast<sal_Int32>(fStepWidth),
                                      static_cast<sal_Int32>(fStepHeight));
            const awt::Size aBetweenSize(static_cast<sal_Int32>(std::max(fBetweenWidth, 0.0)),
                                         static_cast<sal_Int32>(std::max(fBetweenWidth, 0.0)));
            for (sal_Int32 nStep = 0; nStep < nSteps; ++nStep)
            {
                const double fAlong(fLeft + nStep * (fStepWidth + fBetweenWidth));
                const double fX(nIncX == 1 ? fAlong
                                           : rShape->getSize().Width - fAlong - fStepWidth);
                const double fDown(nStep * fStepHeight * fPitch);
                const double fY(nIncY == -1 ? fTop + fHeight - fStepHeight - fDown : fTop + fDown);
                aSteps[nStep]->setPosition(
                    awt::Point(static_cast<sal_Int32>(fX), static_cast<sal_Int32>(fY)));
                aSteps[nStep]->setSize(aStepSize);
                aSteps[nStep]->setChildSize(aStepSize);
                if (nStep + 1 < nSteps && o3tl::make_unsigned(nStep) < aBetween.size())
                {
                    const double fBetweenX(nIncX == 1 ? fX + fStepWidth : fX - fBetweenWidth);
                    const double fNextY(nIncY == -1 ? fY - fStepHeight * fPitch
                                                    : fY + fStepHeight * fPitch);
                    const double fBetweenY((fY + fNextY) / 2.0 + fStepHeight / 2.0
                                           - fBetweenWidth / 2.0);
                    aBetween[nStep]->setPosition(awt::Point(static_cast<sal_Int32>(fBetweenX),
                                                            static_cast<sal_Int32>(fBetweenY)));
                    aBetween[nStep]->setSize(aBetweenSize);
                    aBetween[nStep]->setChildSize(aBetweenSize);
                }
            }
            return;
        }
    }

    if (nCol == 1 && nRow > 1 && !oCellWidth)
    {
        // We have a single column, so count the height based on the parent height, not
        // based on width.
        // Space occurs inside children; also double amount of space is needed outside (on
        // both sides), if the factor comes from a constraint.
        sal_Int32 nNumSpaces = -1;
        if (bSpaceFromConstraints)
            nNumSpaces += 4;
        sal_Int32 nHeight = rShape->getSize().Height / (nRow + (nRow + nNumSpaces) * fRowSpace);

        if (fChildAspectRatio > 1)
        {
            // Shrink width if the aspect ratio requires it.
            nWidth = std::min(rShape->getSize().Width,
                              static_cast<sal_Int32>(nHeight * fChildAspectRatio));
            aChildSize = awt::Size(nWidth, nHeight);
        }
    }

    // The staircase has to fit the room with its steps: a cell is at most the width over the
    // cells of a row with their gaps and the offsets of the rows below the first. A cell whose
    // height came from its width shrinks in step with it.
    if (fAlignOff > 0.0 && nRow > 1)
    {
        const sal_Int32 nRoom(static_cast<sal_Int32>(
            rShape->getSize().Width / (nCol + nGaps * fAlongSpace + (nRow - 1) * fAlignOff)));
        if (nWidth > nRoom && nRoom > 0)
        {
            if (!(nCol == 1) && !oCellHeight)
                aChildSize.Height = static_cast<sal_Int32>(
                    static_cast<double>(aChildSize.Height) * nRoom / nWidth);
            nWidth = nRoom;
            aChildSize.Width = nWidth;
        }
    }

    // A cell the rows make lower than its width asks for keeps its shape and gets narrower with
    // it, so the cells of a row stand side by side with the stated gap between them:
    // Bending_Picture_Caption_List's four squares are 0.1 of a square apart, not spread over the
    // width, and a last row that is not full is centred over cells of that width. The whole
    // stands where horzAlign puts it further down. Cells whose widths the constraints state one
    // by one keep those.
    {
        const bool bWidthsStated(nCount >= 2
                                 && rShape->getChildren()[1]->getDataNodeType() == XML_sibTrans);
        if (fChildAspectRatio && !bWidthsStated && nRow > 0)
        {
            const sal_Int32 nRowHeightRoom(static_cast<sal_Int32>(
                rShape->getSize().Height / (nRow * fContentDown + (nRow - 1) * fRowSpace)));
            const sal_Int32 nOfWidth(static_cast<sal_Int32>(aChildSize.Width / fChildAspectRatio));
            if (nOfWidth > nRowHeightRoom && nRowHeightRoom > 0)
            {
                aChildSize.Height = nRowHeightRoom;
                aChildSize.Width = std::min<sal_Int32>(
                    aChildSize.Width, static_cast<sal_Int32>(nRowHeightRoom * fChildAspectRatio));
            }
        }
    }

    // A flow down the columns has the transitions between the cells of a column and the sp
    // between the columns: Snapshot_Picture_List's composites stand 0.1 of the box the snake
    // states for them apart down a column and 0.1 of their own height apart side by side. The
    // grid is laid out by rows and the cells take it in the other order, so the two gaps change
    // sides for the placing, once the cells have the size the fit of the rows gives them, the
    // drawing's 10914 for the 10925 of that fit.
    if (nFlowDir == XML_col && nBreak != XML_fixed && bTransitionsTakenOut
        && fChildAspectRatio > 0.0)
    {
        const double fDownTheColumn(fAlongSpace * fChildAspectRatio);
        fAlongSpace = fRowSpace / fChildAspectRatio;
        fRowSpace = fDownTheColumn;
    }

    // The flow starts at the top left corner of the box, or at the right or the bottom edge
    // where grDir says so. sp is the gap between the rows and the cells, not a margin at the
    // start: the two gaps that stood at the top before this made Step_Down_Process's rows
    // start far above the box with its sp of less than nothing.
    awt::Point aCurrPos(0, 0);
    if (nIncX == -1)
        aCurrPos.X = rShape->getSize().Width - aChildSize.Width;
    if (nIncY == -1)
        aCurrPos.Y = rShape->getSize().Height - aChildSize.Height;
    // The drawing is laid out in hundredths of a millimetre, 360 EMU, and a step from one cell
    // to the next is kept on that grid, so that every row and every column stands the same
    // whole hundredth from the one before it, whatever the gap's fraction comes to.
    const auto aOnGrid = [](double fStep) {
        return static_cast<sal_Int32>(std::round(fStep / 360.0) * 360.0);
    };

    sal_Int32 nStartX = aCurrPos.X;
    sal_Int32 nColIdx = 0, index = 0;

    const sal_Int32 aContDir
        = rMap.count(XML_contDir) ? rMap.find(XML_contDir)->second : XML_sameDir;

    switch (aContDir)
    {
        case XML_sameDir:
        {
            sal_Int32 nRowHeight = 0;
            // Widths stated one by one share the snake's width out, and a cell the rows then
            // make lower than its share keeps its ar and gets narrower, the transitions with it:
            // Bending_Picture_Accent_List's three composites of ar 0.943 and their transitions of
            // 0.08 share the width as 2508666 each, and the two rows leave them 2519303 high, so
            // they are 0.943 of that wide, as the drawing has them.
            double fShareScale(1.0);
            if (nCount >= 2 && rShape->getChildren()[1]->getDataNodeType() == XML_sibTrans
                && nMaxRowWidth && fChildAspectRatio > 0.0 && nRow > 0)
            {
                const double fNodeWidth(static_cast<double>(rShape->getSize().Width)
                                        * aShapeWidths[0] / nMaxRowWidth);
                const double fRowHeightRoom(rShape->getSize().Height
                                            / (nRow * fContentDown + (nRow - 1) * fRowSpace));
                if (fNodeWidth > 0.0)
                    fShareScale = std::min(1.0, fRowHeightRoom * fChildAspectRatio / fNodeWidth);
            }
            for (auto& aCurrShape : rShape->getChildren())
            {
                awt::Size aCurrSize(aChildSize);
                // aShapeWidths items are a portion of nMaxRowWidth. We want the same ratio,
                // based on the original parent width, ignoring the aspect ratio request.
                bool bWidthsFromConstraints
                    = nCount >= 2 && rShape->getChildren()[1]->getDataNodeType() == XML_sibTrans;
                if (bWidthsFromConstraints && nMaxRowWidth)
                {
                    double fWidthFactor = static_cast<double>(aShapeWidths[index]) / nMaxRowWidth;
                    // We can only work from constraints if spacing is represented by a real
                    // child shape.
                    aCurrSize.Width = rShape->getSize().Width * fWidthFactor * fShareScale;
                }
                if (fChildAspectRatio)
                {
                    aCurrSize.Height = aCurrSize.Width / fChildAspectRatio;

                    // Child shapes are not allowed to leave their parent.
                    aCurrSize.Height = std::min<sal_Int32>(
                        aCurrSize.Height,
                        rShape->getSize().Height / (nRow * fContentDown + (nRow - 1) * fRowSpace));
                }
                if (aCurrSize.Height > nRowHeight)
                {
                    nRowHeight = aCurrSize.Height;
                }
                // a flow from the bottom starts its first row at the box's bottom with the row
                // as high as it comes out, Picture_Accent_Blocks's composites of 2175842 where
                // the cell was guessed at 1409945 above
                if (nIncY == -1 && index < nCol && aCurrSize.Height != aChildSize.Height)
                    aCurrPos.Y = rShape->getSize().Height - aCurrSize.Height;
                aCurrShape->setPosition(aCurrPos);
                aCurrShape->setSize(aCurrSize);
                aCurrShape->setChildSize(aCurrSize);

                index++; // counts index of child, helpful for positioning.

                if (index % nCol == 0 || ((index / nCol) + 1) != nRow)
                    aCurrPos.X += nIncX * aOnGrid(aCurrSize.Width + fAlongSpace * aCurrSize.Width);

                if (++nColIdx == nCol) // condition for next row
                {
                    // if last row, then position children according to number of shapes.
                    if ((index + 1) % nCol != 0 && (index + 1) >= 3
                        && ((index + 1) / nCol + 1) == nRow && nCount != nRow * nCol)
                    {
                        // position first child of last row: a last row that is not full stands
                        // in the middle of the row above it, whatever the widths of its cells,
                        // so it starts half of what it lacks in from the row's start. A row's
                        // extent is its cells with the gap after each but the last.
                        const auto aRowExtent = [&](sal_Int32 nFrom, sal_Int32 nTo) {
                            double fExtent(0.0);
                            for (sal_Int32 i = nFrom; i < nTo; ++i)
                            {
                                const double fWidth(
                                    bWidthsFromConstraints && nMaxRowWidth
                                        ? rShape->getSize().Width
                                              * static_cast<double>(aShapeWidths[i]) / nMaxRowWidth
                                              * fShareScale
                                        : aChildSize.Width);
                                fExtent += fWidth * (i + 1 < nTo ? 1.0 + fAlongSpace : 1.0);
                            }
                            return fExtent;
                        };
                        const double fLacks(aRowExtent(0, nCol)
                                            - aRowExtent(nCol * (nRow - 1), nCount));
                        aCurrPos.X = nStartX + static_cast<sal_Int32>(nIncX * fLacks / 2.0);
                    }
                    else
                        // if not last row, positions first child of that row
                        aCurrPos.X = nStartX;
                    aCurrPos.Y
                        += nIncY * aOnGrid(nRowHeight * fContentDown + fRowSpace * nRowHeight);
                    nColIdx = 0;
                    nRowHeight = 0;
                }

                // positions children in the last row. Every cell of it steps on by its width
                // and the gap, also when the last row is the only one and holds no more than
                // three cells, as the calendars' single row of four days.
                if (index % nCol != 0 && ((index / nCol) + 1) == nRow)
                    aCurrPos.X += nIncX * aOnGrid(aCurrSize.Width + fAlongSpace * aCurrSize.Width);
            }
            break;
        }
        case XML_revDir:
            for (auto& aCurrShape : rShape->getChildren())
            {
                aCurrShape->setPosition(aCurrPos);
                aCurrShape->setSize(aChildSize);
                aCurrShape->setChildSize(aChildSize);

                index++; // counts index of child, helpful for positioning.

                /*
                   index%col -> tests node is at last column
                   ((index/nCol)+1)!=nRow) -> tests node is at last row or not
                   ((index/nCol)+1)%2!=0 -> tests node is at row which is multiple of 2, important for revDir
                   num!=nRow*nCol -> tests how last row nodes should be spread.
                   */

                if ((index % nCol == 0 || ((index / nCol) + 1) != nRow)
                    && ((index / nCol) + 1) % 2 != 0)
                    aCurrPos.X += aOnGrid(aChildSize.Width + fAlongSpace * aChildSize.Width);
                else if (index % nCol != 0
                         && ((index / nCol) + 1) != nRow) // child other than placed at last column
                    aCurrPos.X -= aOnGrid(aChildSize.Width + fAlongSpace * aChildSize.Width);

                if (++nColIdx == nCol) // condition for next row
                {
                    // if last row, then position children according to number of shapes.
                    if ((index + 1) % nCol != 0 && (index + 1) >= 4
                        && ((index + 1) / nCol + 1) == nRow && nCount != nRow * nCol
                        && ((index / nCol) + 1) % 2 == 0)
                        // position first child of last row
                        aCurrPos.X -= aChildSize.Width * 3 / 2;
                    else if ((index + 1) % nCol != 0 && (index + 1) >= 4
                             && ((index + 1) / nCol + 1) == nRow && nCount != nRow * nCol
                             && ((index / nCol) + 1) % 2 != 0)
                        aCurrPos.X = nStartX
                                     + (nIncX * (aChildSize.Width + fAlongSpace * aChildSize.Width)) / 2;
                    else if (((index / nCol) + 1) % 2 != 0)
                        aCurrPos.X = nStartX;

                    aCurrPos.Y
                        += nIncY * aOnGrid(aChildSize.Height + fRowSpace * aChildSize.Height);
                    nColIdx = 0;
                }

                // positions children in the last row.
                if (index % nCol != 0 && index >= 3 && ((index / nCol) + 1) == nRow
                    && ((index / nCol) + 1) % 2 == 0)
                    //if row%2=0 then start from left else
                    aCurrPos.X
                        -= nIncX * aOnGrid(aChildSize.Width + fAlongSpace * aChildSize.Width);
                else if (index % nCol != 0 && index >= 3 && ((index / nCol) + 1) == nRow
                         && ((index / nCol) + 1) % 2 != 0)
                    // start from right
                    aCurrPos.X
                        += nIncX * aOnGrid(aChildSize.Width + fAlongSpace * aChildSize.Width);
            }
            break;
    }

    // With flowDir col the flow runs down a column and goes on at the top of the next one, so
    // the second node stands below the first, not beside it: Snapshot_Picture_List's four
    // pictures stand One over Two and Three over Four. The cells of the grid are the same as
    // for a flow along the rows, the nodes take them in the other order. Only a full grid of
    // cells of one size is turned this way.
    if (nFlowDir == XML_col && nBreak != XML_fixed && nCol > 1 && nRow > 1
        && nCount == nCol * nRow
        && !(nCount >= 2 && rShape->getChildren()[1]->getDataNodeType() == XML_sibTrans))
    {
        std::vector<awt::Point> aByRows;
        for (const auto& rChild : rShape->getChildren())
            aByRows.push_back(rChild->getPosition());
        sal_Int32 nIndex(0);
        for (auto& rChild : rShape->getChildren())
        {
            const sal_Int32 nRowOf(nIndex % nRow);
            const sal_Int32 nColumnOf(nIndex / nRow);
            rChild->setPosition(aByRows[nRowOf * nCol + nColumnOf]);
            ++nIndex;
        }
    }

    // In the centre mode the whole stands where horzAlign puts it in the width the cells left
    // room in, the middle where it says nothing, the right edge for r, and stays at the left for
    // l, List_Dots' row of dots: the widest row against the snake's width, and the same shift for
    // every row, so a last row already in the middle of the one above it stays there.
    const sal_Int32 nHorzAlign(rMap.count(XML_horzAlign) ? rMap.find(XML_horzAlign)->second
                                                          : XML_ctr);
    if (!bOffsetMode && nHorzAlign != XML_l && !rShape->getChildren().empty())
    {
        sal_Int32 nLeft(std::numeric_limits<sal_Int32>::max());
        sal_Int32 nRight(std::numeric_limits<sal_Int32>::min());
        for (const auto& rChild : rShape->getChildren())
        {
            nLeft = std::min(nLeft, rChild->getPosition().X);
            nRight = std::max(nRight, rChild->getPosition().X + rChild->getSize().Width);
        }
        const sal_Int32 nRoom(rShape->getSize().Width - (nRight - nLeft));
        const sal_Int32 nShift((nHorzAlign == XML_r ? nRoom : nRoom / 2) - nLeft);
        if (nShift > 0)
            for (auto& rChild : rShape->getChildren())
            {
                awt::Point aPosition(rChild->getPosition());
                aPosition.X += nShift;
                rChild->setPosition(aPosition);
            }
    }

    // The rows step aside now, each by its number of offsets.
    if (bOffsetMode)
    {
        if (fAlignOff != 0.0)
        {
            sal_Int32 nIndex(0);
            for (auto& rChild : rShape->getChildren())
            {
                const sal_Int32 nRowOf(nIndex / nCol);
                awt::Point aPosition(rChild->getPosition());
                aPosition.X += static_cast<sal_Int32>(nIncX * nRowOf * fAlignOff
                                                      * rChild->getSize().Width);
                rChild->setPosition(aPosition);
                ++nIndex;
            }
        }
    }
}

// A pyramid is a stack of levels, one per node, each a row of the whole width and its share of
// the height, the first node's level at the top either way, and linDir says where the base is:
// fromB makes the last node's level the widest, Basic_Pyramid standing on its base, and fromT
// the first node's, Inverted_Pyramid hanging from it. Each node's shape is a composite whose
// children the pyramid itself sizes: the level, pyraLvlNode, is a trapezoid as wide as the
// pyramid times its count from the apex over the number of levels, in the middle of the
// pyramid's width, and where an accent stands beside the pyramid, pyraAcctBkgdNode and
// pyraAcctTxNode, the pyramid is the width less pyraAcctRatio of it, 0.68 of 22578 in units of
// 360 EMU on Basic_Pyramid, its levels 3838, 7676, 11515 and 15353 for four nodes. The accent
// runs from the pyramid's axis, moved out by the half step of each level below the apex, to the
// far edge, 14901, 12982 and 9144 wide there, and it slants along the pyramid's edge, a
// non-isosceles trapezoid with the slanted side's inset 0.51 of its width, the drawing's number,
// turned by a half turn where the levels grow downwards. The composite leaves what the pyramid
// has placed as it is; its own constraints for those children, "w val 1", are place holders.
namespace
{
rtl::Reference<svx::diagram::Point> firstDataNodeOf(const SmartArtDiagram& rDgm,
                                                    const ShapePtr& rShape);
std::vector<Constraint> collectDirectConstraints(const LayoutNode& rLayoutNode);
void gatherOwnConstraints(const LayoutAtom& rAtom, std::vector<Constraint>& rOut);
}

void PyraAlg::layoutShapeChildren(const SmartArtDiagram& rDgm, AlgAtom& rAlg,
                                  const ShapePtr& rShape,
                                  const std::vector<Constraint>& rConstraints)
{
    if (rShape->getChildren().empty() || rShape->getSize().Width == 0
        || rShape->getSize().Height == 0)
        return;

    const AlgAtom::ParamMap& rMap(rAlg.getMap());
    const sal_Int32 nDirection(rMap.count(XML_linDir) ? rMap.find(XML_linDir)->second
                                                      : XML_fromT);
    const sal_Int32 nAccentSide(rMap.count(XML_pyraAcctPos) ? rMap.find(XML_pyraAcctPos)->second
                                                            : XML_bef);
    const auto aNamed = [&rAlg](sal_Int32 nType, const OUString& rDefault) {
        const OUString aName(rAlg.getNamedParam(nType));
        return aName.isEmpty() ? rDefault : aName;
    };
    const OUString aLevelName(aNamed(XML_pyraLvlNode, u"level"_ustr));
    const OUString aAccentName(aNamed(XML_pyraAcctBkgdNode, u"acctBkgd"_ustr));
    const OUString aAccentTextName(aNamed(XML_pyraAcctTxNode, u"acctTx"_ustr));

    // the accent's part of the width, 0.33 where nothing is said, and only where a node has an
    // accent shape at all; the ratio stands in a choose of the node's own, "pyraAcctRatio val
    // 0.32" where the nodes have children to show, so the node's own decided constraints are
    // read for it
    double fAccentRatio(0.33);
    {
        rtl::Reference<svx::diagram::Point> xOwn;
        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
            if (rEntry.second == rShape)
            {
                xOwn = rEntry.first;
                break;
            }
        std::vector<Constraint> aOwn(rConstraints);
        gatherDecidedConstraints(rDgm, rAlg.getLayoutNode(), xOwn, aOwn);
        for (const Constraint& rConstraint : aOwn)
            if (rConstraint.mnType == XML_pyraAcctRatio && rConstraint.mfValue > 0.0)
                fAccentRatio = rConstraint.mfValue;
    }
    bool bAccent(false);
    for (const ShapePtr& pLevel : rShape->getChildren())
        for (const ShapePtr& pPart : pLevel->getChildren())
            if (pPart->getInternalName() == aAccentName)
                bAccent = true;
    if (!bAccent)
        fAccentRatio = 0.0;

    const sal_Int32 nCount(rShape->getChildren().size());
    const double fWidth(rShape->getSize().Width);
    const double fHeight(rShape->getSize().Height);
    const double fRow(fHeight / nCount);
    const double fPyramid(fWidth * (1.0 - fAccentRatio));
    const bool bAccentAfter(nAccentSide != XML_bef);
    const double fPyramidLeft(bAccentAfter ? 0.0 : fWidth - fPyramid);
    const double fAxis(fPyramidLeft + fPyramid / 2.0);

    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    sal_Int32 nLevel(0);
    for (const ShapePtr& pLevel : rShape->getChildren())
    {
        // the level's count from the apex, the last node's level being the base for fromB
        const sal_Int32 nFromApex(nDirection == XML_fromB ? nLevel + 1 : nCount - nLevel);
        const double fLevelWidth(fPyramid * nFromApex / nCount);
        const double fTop(nLevel * fRow);
        const awt::Size aRowSize(static_cast<sal_Int32>(fWidth), static_cast<sal_Int32>(fRow));
        pLevel->setPosition(awt::Point(0, static_cast<sal_Int32>(fTop)));
        pLevel->setSize(aRowSize);
        pLevel->setChildSize(aRowSize);

        for (const ShapePtr& pPart : pLevel->getChildren())
        {
            const OUString& rName(pPart->getInternalName());
            if (rName == aLevelName)
            {
                const awt::Size aSize(static_cast<sal_Int32>(fLevelWidth),
                                      static_cast<sal_Int32>(fRow));
                pPart->setPosition(
                    awt::Point(static_cast<sal_Int32>(fAxis - fLevelWidth / 2.0), 0));
                pPart->setSize(aSize);
                pPart->setChildSize(aSize);
                rMutable.getLaidOutSideways().insert(pPart.get());
            }
            else if (rName == aAccentName || rName == aAccentTextName)
            {
                // from the axis, moved out by the half step of each level below the apex,
                // which is the edge of the level above, to the far edge
                const double fOut((nFromApex - 1) * fPyramid / (2.0 * nCount));
                const double fLeft(bAccentAfter ? fAxis + fOut : 0.0);
                const double fRight(bAccentAfter ? fWidth : fAxis - fOut);
                const awt::Size aSize(static_cast<sal_Int32>(fRight - fLeft),
                                      static_cast<sal_Int32>(fRow));
                pPart->setPosition(awt::Point(static_cast<sal_Int32>(fLeft), 0));
                pPart->setSize(aSize);
                pPart->setChildSize(aSize);
                if (pPart->getCustomShapeProperties()->getShapePresetType()
                    == XML_nonIsoscelesTrapezoid)
                {
                    // the side towards the pyramid slants; with the levels growing downwards
                    // the shape is turned by a half turn and the other inset takes the slant
                    const bool bDownwards(nDirection == XML_fromB);
                    const bool bSlantLeft(bAccentAfter != bDownwards);
                    auto& rGuides(pPart->getCustomShapeProperties()->getAdjustmentGuideList());
                    rGuides.push_back({ u"adj1"_ustr, bSlantLeft ? u"51000"_ustr : u"0"_ustr });
                    rGuides.push_back({ u"adj2"_ustr, bSlantLeft ? u"0"_ustr : u"51000"_ustr });
                    if (bDownwards)
                        pPart->setRotation(180 * PER_DEGREE);
                }
                rMutable.getLaidOutSideways().insert(pPart.get());
            }
        }
        ++nLevel;
    }
}

bool CompositeAlg::inferFromLayoutProperty(const LayoutProperty& rMap, sal_Int32 nRefType,
                                           sal_Int32& rValue)
{
    // An edge or a middle that the constraints have not stated outright follows from the two of
    // its axis that they have: the left from the right and the width, the bottom from the top and
    // the height, and so on.
    const auto aGet = [&rMap](sal_Int32 nWhat, sal_Int32& rOut) {
        const auto aFound = rMap.find(nWhat);
        if (aFound == rMap.end())
            return false;
        rOut = aFound->second;
        return true;
    };

    sal_Int32 nA(0), nB(0);
    switch (nRefType)
    {
        case XML_r:
            if (aGet(XML_l, nA) && aGet(XML_w, nB)) { rValue = nA + nB; return true; }
            if (aGet(XML_ctrX, nA) && aGet(XML_w, nB)) { rValue = nA + nB / 2; return true; }
            return false;
        case XML_l:
            if (aGet(XML_r, nA) && aGet(XML_w, nB)) { rValue = nA - nB; return true; }
            if (aGet(XML_ctrX, nA) && aGet(XML_w, nB)) { rValue = nA - nB / 2; return true; }
            return false;
        case XML_ctrX:
            if (aGet(XML_l, nA) && aGet(XML_w, nB)) { rValue = nA + nB / 2; return true; }
            if (aGet(XML_r, nA) && aGet(XML_w, nB)) { rValue = nA - nB / 2; return true; }
            return false;
        case XML_b:
            if (aGet(XML_t, nA) && aGet(XML_h, nB)) { rValue = nA + nB; return true; }
            if (aGet(XML_ctrY, nA) && aGet(XML_h, nB)) { rValue = nA + nB / 2; return true; }
            return false;
        case XML_t:
            if (aGet(XML_b, nA) && aGet(XML_h, nB)) { rValue = nA - nB; return true; }
            if (aGet(XML_ctrY, nA) && aGet(XML_h, nB)) { rValue = nA - nB / 2; return true; }
            return false;
        case XML_ctrY:
            if (aGet(XML_t, nA) && aGet(XML_h, nB)) { rValue = nA + nB / 2; return true; }
            if (aGet(XML_b, nA) && aGet(XML_h, nB)) { rValue = nA - nB / 2; return true; }
            return false;
        case XML_w:
            if (aGet(XML_l, nA) && aGet(XML_r, nB)) { rValue = nB - nA; return true; }
            return false;
        case XML_h:
            if (aGet(XML_t, nA) && aGet(XML_b, nB)) { rValue = nB - nA; return true; }
            return false;
        default:
            return false;
    }
}

// The size of a shape laid out already, by the name of its layout node, w or h, in EMU. Every
// shape of that name has the same size in the layouts that name one this way, so the first with a
// size is taken, the first in the order of the data's points: the map of the presentation
// points is ordered by their addresses, which differ from one build to the next, and while a
// row fits its children the shapes of one name can hold sizes of different rounds, so Sub-Step_
// Process's chLin1 of 1.38 of parTx1 came out a percent apart on two builds of one source.
// Nothing where none is laid out yet.
static std::optional<sal_Int32> sizeOfLaidOutShape(const SmartArtDiagram& rDgm,
                                                   std::u16string_view rName, sal_Int32 nWhat)
{
    const auto& rShapes(rDgm.getLayout()->getPresPointShapeMap());
    for (const rtl::Reference<svx::diagram::Point>& rPoint : rDgm.getData()->getPoints())
    {
        const auto aEntry = rShapes.find(rPoint);
        if (aEntry == rShapes.end())
            continue;
        const ShapePtr& pShape(aEntry->second);
        if (!pShape || pShape->getInternalName() != rName || pShape->getSize().Width <= 0
            || pShape->getSize().Height <= 0)
            continue;
        return nWhat == XML_w ? pShape->getSize().Width : pShape->getSize().Height;
    }
    return std::nullopt;
}

// The value of a name of the layout's own, userA and the rest, in EMU: what a node above worked
// out, or, where the statement was kept because it refers to a shape levels down, that shape's
// size now that it is laid out, or the spacing it refers to, sp or sibSp, which rAll states as a
// part of a named shape's size in turn. Nothing where it cannot be settled yet.
static std::optional<sal_Int32> resolveUserVariable(const SmartArtDiagram& rDgm, sal_Int32 nType,
                                                    const std::vector<Constraint>& rAll)
{
    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    const auto aKnown = rMutable.getUserVariables().find(nType);
    if (aKnown != rMutable.getUserVariables().end())
        return aKnown->second;

    const auto aDeferred = rMutable.getDeferredUserVariables().find(nType);
    if (aDeferred == rMutable.getDeferredUserVariables().end())
        return std::nullopt;
    const SmartArtDiagram::DeferredUserVariable& rStated(aDeferred->second);

    std::optional<sal_Int32> aValue;
    if (rStated.mnRefType == XML_w || rStated.mnRefType == XML_h)
        aValue = sizeOfLaidOutShape(rDgm, rStated.msShapeName, rStated.mnRefType);
    else if (rStated.mnRefType == XML_sp || rStated.mnRefType == XML_sibSp)
    {
        for (const Constraint& rSpacing : rAll)
        {
            if (rSpacing.mnType != rStated.mnRefType
                || (rSpacing.mnRefType != XML_w && rSpacing.mnRefType != XML_h)
                || rSpacing.msRefForName.isEmpty())
                continue;
            const std::optional<sal_Int32> aOf(
                sizeOfLaidOutShape(rDgm, rSpacing.msRefForName, rSpacing.mnRefType));
            if (aOf)
                aValue = static_cast<sal_Int32>(
                    *aOf * (rSpacing.mfFactor > 0.0 ? rSpacing.mfFactor : 1.0));
            break;
        }
    }
    if (!aValue)
        return std::nullopt;

    const sal_Int32 nValue(static_cast<sal_Int32>(*aValue * rStated.mfFactor));
    rMutable.getUserVariables()[nType] = nValue;
    return nValue;
}

static double fontOfGroup(const SmartArtDiagram& rDgm, const std::vector<Constraint>& rConstraints,
                          const OUString& rName, OUString& rGroup);

void CompositeAlg::applyConstraintToLayout(const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                                           const std::vector<Constraint>& rAll,
                                           const Constraint& rConstraint,
                                           LayoutPropertyMap& rProperties,
                                           std::map<sal_Int32, sal_Int32>& rUserVariables)
{
    // A name stated as a part of the size of a shape levels down, refFor des with a name that is
    // no child of this node, cannot be worked out here: the constraints for that shape ride along
    // as well and would leave an entry under its name against this node's size, which is not the
    // shape's size. The same holds for a name stated as a spacing, sp or sibSp, which is no size
    // of this node. The statement is kept, and the value is read off the shape when a node below
    // takes the name.
    if (isUserVariable(rConstraint.mnType))
    {
        const bool bSpacing(rConstraint.mnRefType == XML_sp || rConstraint.mnRefType == XML_sibSp);
        bool bOfAShapeBelow(false);
        if (rConstraint.mnRefFor == XML_des && !rConstraint.msRefForName.isEmpty()
            && (rConstraint.mnRefType == XML_w || rConstraint.mnRefType == XML_h))
        {
            bOfAShapeBelow = true;
            for (const ShapePtr& pChild : rShape->getChildren())
                if (pChild->getInternalName() == rConstraint.msRefForName)
                    bOfAShapeBelow = false;
        }
        if (bSpacing || bOfAShapeBelow)
        {
            const_cast<SmartArtDiagram&>(rDgm).getDeferredUserVariables()[rConstraint.mnType]
                = { rConstraint.msRefForName, rConstraint.mnRefType,
                    rConstraint.mfFactor > 0.0 ? rConstraint.mfFactor : 1.0 };
            return;
        }
    }

    // A value stated under a name of its own holds for everything below the node that states it,
    // and each of those nodes states the bare name again to say that it takes it. Such a bare
    // statement carries no reference and must not overwrite what came from above. The statement
    // itself rides down with the constraints as well, and it is worked out once, at the node that
    // states it, against that node's own size: further down it is taken like the bare name is,
    // for it would come out against the size of every node below in turn otherwise.
    const bool bTakesWhatIsKnown(
        isUserVariable(rConstraint.mnType)
        && ((rConstraint.mnRefType == XML_none && rConstraint.mfValue == 0.0)
            || (rConstraint.mnFor == XML_des
                && rUserVariables.find(rConstraint.mnType) != rUserVariables.end())));
    if (bTakesWhatIsKnown)
    {
        const std::optional<sal_Int32> aValue(
            resolveUserVariable(rDgm, rConstraint.mnType, rAll));
        if (aValue)
        {
            rUserVariables[rConstraint.mnType] = *aValue;
            rProperties[u""_ustr][rConstraint.mnType] = *aValue;
        }
        return;
    }

    // A reference to such a name reads it from the node above that worked it out, where this one
    // holds no value of its own for it.
    if (isUserVariable(rConstraint.mnRefType) && !rConstraint.msForName.isEmpty())
    {
        const LayoutPropertyMap::const_iterator aHere = rProperties.find(u""_ustr);
        const bool bHere(aHere != rProperties.end()
                         && aHere->second.find(rConstraint.mnRefType) != aHere->second.end());
        if (!bHere)
        {
            const auto aKnown = rUserVariables.find(rConstraint.mnRefType);
            if (aKnown != rUserVariables.end())
            {
                rProperties[rConstraint.msForName][rConstraint.mnType]
                    = aKnown->second * rConstraint.mfFactor;
                return;
            }
        }
    }

    // A constraint that states a value under a name of its own, userA and the rest, names no
    // shape. It belongs to the shape that holds it, which is the one under the empty name, and
    // the constraints that go on to refer to it read it from there.
    // TODO handle the case when we have ptType="...", not forName="...".
    if (rConstraint.msForName.isEmpty() && !isUserVariable(rConstraint.mnType))
    {
        return;
    }

    const LayoutPropertyMap::const_iterator aRef = rProperties.find(rConstraint.msRefForName);
    if (aRef == rProperties.end())
        return;

    // no refType but a named shape: take the same type of that shape, as in "h refFor ch
    // refForName child op equ"
    const sal_Int32 nRefType(
        rConstraint.mnRefType == XML_none && !rConstraint.msRefForName.isEmpty()
                && rConstraint.mfValue == 0.0
            ? rConstraint.mnType
            : rConstraint.mnRefType);
    const LayoutProperty::const_iterator aRefType = aRef->second.find(nRefType);
    sal_Int32 nInferredValue = 0;
    if (aRefType != aRef->second.end())
    {
        // Reference is found directly.
        const sal_Int32 nValue(aRefType->second * rConstraint.mfFactor);
        rProperties[rConstraint.msForName][rConstraint.mnType] = nValue;
        if (isUserVariable(rConstraint.mnType))
            rUserVariables[rConstraint.mnType] = nValue;
    }
    else if (inferFromLayoutProperty(aRef->second, nRefType, nInferredValue))
    {
        // Reference can be inferred. A name of the layout's own worked out this way, userC as
        // the right edge of a shape that states its left and its width, is a value like one
        // read directly, and holds for everything below.
        const sal_Int32 nValue(nInferredValue * rConstraint.mfFactor);
        rProperties[rConstraint.msForName][rConstraint.mnType] = nValue;
        if (isUserVariable(rConstraint.mnType))
            rUserVariables[rConstraint.mnType] = nValue;
    }
    else
    {
        // Reference not found, assume a fixed value.
        // Values are never in EMU, while oox::drawingml::Shape position and size are always in
        // EMU. A stated font is the font of this pass for the named node's group, the stated
        // size on the first, so what is stated as a part of it, Numbered_List's userA of half
        // desTx's font, follows the font the texts are fitted with.
        double fStated(rConstraint.mfValue);
        if (rConstraint.mnType == XML_primFontSz && rConstraint.mnRefType == XML_none
            && !rConstraint.msForName.isEmpty())
        {
            OUString aGroup;
            fStated = fontOfGroup(rDgm, rAll, rConstraint.msForName, aGroup);
        }
        const double fValue = o3tl::convert(fStated,
                                            isFontUnit(rConstraint.mnRefType) ? o3tl::Length::pt
                                                                              : o3tl::Length::mm,
                                            o3tl::Length::emu);
        rProperties[rConstraint.msForName][rConstraint.mnType] = fValue;
    }
}


// width of the Diagram's text font as a part of the width of the stand-in, Liberation Sans
static double textWidthPartFor(std::u16string_view rTextFont)
{
    if (rTextFont == u"Arial" || rTextFont == u"Liberation Sans")
        return 0.9928;
    return 0.969;
}

// The height the text rParagraphs takes at fFontPt points in a box nWidthEmu wide, in EMU,
// laid out by the edit engine with the line spacing and the space after a paragraph as parts of
// the font height.
// The left indent of a bullet paragraph of a Diagram's text, in EMU, by its font in points: the
// drawings carry it in steps of 57150, one up to 11 points, two up to 15, three up to 19, four
// up to 27 and five up to 41; what comes above 41 is not read yet and taken as six.
static sal_Int64 bulletIndentFor(double fFontPt)
{
    sal_Int64 nSteps(6);
    if (fFontPt < 12.0)
        nSteps = 1;
    else if (fFontPt < 16.0)
        nSteps = 2;
    else if (fFontPt < 20.0)
        nSteps = 3;
    else if (fFontPt < 28.0)
        nSteps = 4;
    else if (fFontPt < 42.0)
        nSteps = 5;
    return 57150 * nSteps;
}

// The space after a paragraph of a Diagram's text, as parts of its font: first after a
// first-level paragraph, second after a deeper one. 0.35 and 0.15 where the text algorithm of the
// node states nothing, and its "lnSpAfParP" and "lnSpAfChP" in percent where it does, Vertical_
// Header_List's 20 after the deeper ones.
typedef std::pair<double, double> SpaceAfterParts;

// put rParagraphs into rEngine as the Diagram's text at fFontPt of font rTextFont, measured
// with the stand-in, paragraph sizes from rFontOfParagraph, the space after from rSpaceAfter,
// on a paper nWidthEmu (EMU) wide and of no bounded height
static void setUpTextEngine(EditEngine& rEngine, const std::vector<OUString>& rParagraphs,
                            sal_Int64 nWidthEmu, double fFontPt, std::u16string_view rTextFont,
                            const std::vector<double>& rFontOfParagraph,
                            const std::vector<sal_Int32>& rLevelOfParagraph,
                            const SpaceAfterParts& rSpaceAfter = SpaceAfterParts(0.35, 0.15))
{
    // an Aptos line is a bit higher than the line of the stand-in, the proportional spacing
    // brings it up to that; an Arial line is its own, the space after a paragraph a part of it
    const sal_uInt16 nLineSpacingPercent(103);
    const bool bArialLine(rTextFont == u"Arial" || rTextFont == u"Liberation Sans");
    // the width rounded up to the hundredth of a millimetre, so that a line measured as exactly
    // this wide still stands on one line; the paper is wider by the part the stand-in is wider
    // than the drawings' font, so the stand-in breaks its lines where that font does
    rEngine.SetPaperSize(Size(
        static_cast<tools::Long>(std::min<double>(
            (static_cast<double>(nWidthEmu) / textWidthPartFor(rTextFont) + 359) / 360,
            0x7fffffff)),
        0x7fffffff));
    SfxItemSet aSet(rEngine.GetEmptyItemSet());
    aSet.Put(SvxFontItem(FAMILY_SWISS, u"Liberation Sans"_ustr, OUString(), PITCH_VARIABLE,
                         RTL_TEXTENCODING_UNICODE, EE_CHAR_FONTINFO));
    if (!bArialLine)
    {
        SvxLineSpacingItem aSpacing(LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL);
        aSpacing.SetPropLineSpace(nLineSpacingPercent);
        aSet.Put(aSpacing);
    }
    OUStringBuffer aText;
    for (size_t n = 0; n < rParagraphs.size(); ++n)
    {
        if (n)
            aText.append('\n');
        aText.append(rParagraphs[n]);
    }
    rEngine.SetText(aText.makeStringAndClear());
    const sal_Int32 nParagraphs(rEngine.GetParagraphCount());
    for (sal_Int32 nParagraph = 0; nParagraph < nParagraphs; ++nParagraph)
    {
        // a paragraph of its own size, the deeper levels smaller, and its space after, none
        // after the last
        const size_t nIndex(static_cast<size_t>(nParagraph));
        const double fOwnFont(nIndex < rFontOfParagraph.size() && rFontOfParagraph[nIndex] > 0.0
                                  ? rFontOfParagraph[nIndex]
                                  : fFontPt);
        const sal_Int32 nLevel(nIndex < rLevelOfParagraph.size() ? rLevelOfParagraph[nIndex] : 1);
        SfxItemSet aOwn(aSet);
        aOwn.Put(SvxFontHeightItem(static_cast<sal_uInt32>(fOwnFont * 2540.0 / 72.0), 100,
                                   EE_CHAR_FONTHEIGHT));
        const double fArialLine(1.0594);
        const double fAfter((nParagraph + 1 == nParagraphs
                                 ? 0.0
                                 : (nLevel <= 1 ? rSpaceAfter.first : rSpaceAfter.second))
                            * (bArialLine ? fArialLine : 1.0));
        if (bArialLine)
        {
            SvxLineSpacingItem aLine(LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL);
            aLine.SetLineHeight(static_cast<sal_uInt16>(fArialLine * fOwnFont * 2540.0 / 72.0));
            aLine.SetLineSpaceRule(SvxLineSpaceRule::Fix);
            aLine.SetInterLineSpaceRule(SvxInterLineSpaceRule::Off);
            aOwn.Put(aLine);
        }
        aOwn.Put(SvxULSpaceItem(0, static_cast<sal_uInt16>(fAfter * fOwnFont * 2540.0 / 72.0),
                                EE_PARA_ULSPACE));
        // a paragraph below the first level carries a bullet in a hanging indent, in the
        // hundredths of a millimetre the engine measures in, and widened as the paper is, so
        // that it keeps its own length against the text
        if (nLevel >= 2)
        {
            SvxLRSpaceItem aIndent(EE_PARA_LRSPACE);
            aIndent.SetTextLeft(SvxIndentValue::twips(
                o3tl::convert(bulletIndentFor(fOwnFont) / textWidthPartFor(rTextFont),
                              o3tl::Length::emu,
                              o3tl::Length::mm100)));
            aOwn.Put(aIndent);
        }
        rEngine.SetParaAttribs(nParagraph, aOwn);
    }
}

// The height the paragraphs rParagraphs take as the Diagram's text at fFontPt points of the font
// rFontName in a box nWidthEmu wide, in EMU, each paragraph at its own size from rFontOfParagraph
// and with the space after its level from rLevelOfParagraph and rSpaceAfter where given.
static sal_Int64 measureTextHeight(const std::vector<OUString>& rParagraphs, sal_Int64 nWidthEmu,
                                   double fFontPt, std::u16string_view rFontName,
                                   const std::vector<double>& rFontOfParagraph = {},
                                   const std::vector<sal_Int32>& rLevelOfParagraph = {},
                                   const SpaceAfterParts& rSpaceAfter
                                   = SpaceAfterParts(0.35, 0.15))
{
    if (rParagraphs.empty() || nWidthEmu <= 0 || fFontPt <= 0.0)
        return 0;
    static rtl::Reference<SfxItemPool> xPool(EditEngine::CreatePool());
    ScopedVclPtrInstance<VirtualDevice> pDevice;
    pDevice->SetMapMode(MapMode(MapUnit::Map100thMM));
    EditEngine aEngine(xPool.get());
    aEngine.SetRefDevice(pDevice.get());
    setUpTextEngine(aEngine, rParagraphs, nWidthEmu, fFontPt, rFontName, rFontOfParagraph,
                    rLevelOfParagraph, rSpaceAfter);
    return static_cast<sal_Int64>(aEngine.GetTextHeight()) * 360;
}

// The inset of the text area of the shape rShape on each side, as a part of its shorter side:
// a roundRect holds its text 0.29289 of its corner's radius in, the radius its adjustment, a part
// of the shorter side, 1/6 where not stated; Vertical_Bullet_List's parent text of 1843 has 1663
// of room in it. An ellipse holds its text in the rectangle inside it. 0 for a shape whose
// text has the whole of it.
static double textAreaInsetPart(const ShapePtr& rShape)
{
    if (!rShape || !rShape->getCustomShapeProperties())
        return 0.0;
    if (rShape->getCustomShapeProperties()->getShapePresetType() == XML_ellipse)
        return (1.0 - std::sqrt(0.5)) / 2.0;
    if (rShape->getCustomShapeProperties()->getShapePresetType() != XML_roundRect)
        return 0.0;
    double fRadius(1.0 / 6.0);
    for (const CustomShapeGuide& rGuide :
         rShape->getCustomShapeProperties()->getAdjustmentGuideList())
        if (rGuide.maName == "adj" || rGuide.maName == "adj1")
        {
            const OUString aValue(rGuide.maFormula.startsWith("val ")
                                      ? rGuide.maFormula.copy(4)
                                      : rGuide.maFormula);
            fRadius = std::clamp(aValue.toDouble() / 100000.0, 0.0, 0.5);
        }
    return 0.29289 * fRadius;
}

// parts of the box a preset keeps free of text, left, top, right and bottom, after the
// preset's own formulas: an arrow holds its text in its stem, a chevron between notch and
// point. Nothing for a preset that uses the whole box or is handled by textAreaInsetPart
static std::optional<std::array<double, 4>> textAreaPartsOf(const ShapePtr& rShape,
                                                             double fWidth, double fHeight)
{
    if (!rShape || !rShape->getCustomShapeProperties() || fWidth <= 0.0 || fHeight <= 0.0)
        return std::nullopt;
    const sal_Int32 nType(rShape->getCustomShapeProperties()->getShapePresetType());
    if (nType != XML_downArrow && nType != XML_upArrow && nType != XML_rightArrow
        && nType != XML_leftArrow && nType != XML_chevron && nType != XML_homePlate)
        return std::nullopt;
    double fFirst(50000.0), fSecond(50000.0);
    for (const CustomShapeGuide& rGuide :
         rShape->getCustomShapeProperties()->getAdjustmentGuideList())
    {
        const OUString aValue(rGuide.maFormula.startsWith("val ") ? rGuide.maFormula.copy(4)
                                                                  : rGuide.maFormula);
        if (rGuide.maName == "adj1" || rGuide.maName == "adj")
            fFirst = aValue.toDouble();
        else if (rGuide.maName == "adj2")
            fSecond = aValue.toDouble();
    }
    const double fShorter(std::min(fWidth, fHeight));
    // chevron and homePlate: the text stands between notch and point
    if (nType == XML_chevron || nType == XML_homePlate)
    {
        const double fPoint(std::clamp(fFirst, 0.0, 100000.0 * fWidth / fShorter) / 100000.0
                            * fShorter);
        if (nType == XML_homePlate)
            return std::array<double, 4>{ 0.0, 0.0, fPoint / 2.0 / fWidth, 0.0 };
        if (fWidth - 2.0 * fPoint <= 0.0)
            return std::nullopt;
        return std::array<double, 4>{ fPoint / fWidth, 0.0, fPoint / fWidth, 0.0 };
    }
    const bool bUpDown(nType == XML_downArrow || nType == XML_upArrow);
    // stem across and head along the arrow, both from the adjustments
    const double fAcross(bUpDown ? fWidth : fHeight);
    const double fAlong(bUpDown ? fHeight : fWidth);
    const double fStemPart(std::clamp(fFirst, 0.0, 100000.0) / 100000.0);
    const double fHead(
        std::clamp(fSecond, 0.0, 100000.0 * fAlong / fShorter) / 100000.0 * fShorter);
    const double fStemSide((fAcross - fAcross * fStemPart) / 2.0);
    // the text reaches into the head up to where stem and head meet
    const double fIntoHead(fStemSide * fHead / (fAcross / 2.0));
    const double fAcrossPart(fStemSide / fAcross);
    const double fAlongPart(std::max(0.0, fHead - fIntoHead) / fAlong);
    switch (nType)
    {
        case XML_downArrow:
            return std::array<double, 4>{ fAcrossPart, 0.0, fAcrossPart, fAlongPart };
        case XML_upArrow:
            return std::array<double, 4>{ fAcrossPart, fAlongPart, fAcrossPart, 0.0 };
        case XML_rightArrow:
            return std::array<double, 4>{ 0.0, fAcrossPart, fAlongPart, fAcrossPart };
        default:
            return std::array<double, 4>{ fAlongPart, fAcrossPart, 0.0, fAcrossPart };
    }
}

// The height the paragraphs rParagraphs take in the shape rShape nWidthEmu wide, in EMU, as
// measureTextHeight measures them, with the inset of the shape's text area over and under the
// text added and taken off the width too; nOverUnderEmu are the margins over and under the text,
// which make the shape higher and with that the inset where the height is the shorter side. The
// inset on each side goes to pInsetEmu where given, 0 for a shape whose text has the whole of it.
static sal_Int64 measureTextHeightInShape(const ShapePtr& rShape,
                                          const std::vector<OUString>& rParagraphs,
                                          sal_Int64 nWidthEmu, sal_Int64 nOverUnderEmu,
                                          double fFontPt,
                                          const std::vector<double>& rFontOfParagraph,
                                          const std::vector<sal_Int32>& rLevelOfParagraph,
                                          const SpaceAfterParts& rSpaceAfter,
                                          std::u16string_view rTextFont,
                                          sal_Int64* pInsetEmu = nullptr)
{
    const double fPart(textAreaInsetPart(rShape));
    if (pInsetEmu)
        *pInsetEmu = 0;
    if (fPart <= 0.0)
        return measureTextHeight(rParagraphs, nWidthEmu, fFontPt, rTextFont, rFontOfParagraph,
                                 rLevelOfParagraph, rSpaceAfter);
    // the inset follows the shorter side, which is the height the text makes where the shape
    // is wider than that, so a few rounds settle it
    sal_Int64 nInset(static_cast<sal_Int64>(fPart * nWidthEmu));
    sal_Int64 nText(0);
    for (int nRound = 0; nRound < 4; ++nRound)
    {
        nText = measureTextHeight(rParagraphs, std::max<sal_Int64>(1, nWidthEmu - 2 * nInset),
                                  fFontPt, rTextFont, rFontOfParagraph, rLevelOfParagraph,
                                  rSpaceAfter);
        const double fHeight(static_cast<double>(nText + nOverUnderEmu) / (1.0 - 2.0 * fPart));
        nInset = static_cast<sal_Int64>(fPart * std::min<double>(nWidthEmu, fHeight));
    }
    if (pInsetEmu)
        *pInsetEmu = nInset;
    return nText + 2 * nInset;
}

// The width of the widest of the paragraphs rParagraphs on one line, in EMU, measured by the
// same engine that measures the height, so that a box of this width holds each on one line; the
// stand-in's width is taken down to the width of the drawings' font.
static sal_Int64 measureWidestLine(const std::vector<OUString>& rParagraphs, double fFontPt,
                                   std::u16string_view rFontName,
                                   const std::vector<double>& rFontOfParagraph = {},
                                   const std::vector<sal_Int32>& rLevelOfParagraph = {})
{
    if (rParagraphs.empty() || fFontPt <= 0.0)
        return 0;
    static rtl::Reference<SfxItemPool> xPool(EditEngine::CreatePool());
    ScopedVclPtrInstance<VirtualDevice> pDevice;
    pDevice->SetMapMode(MapMode(MapUnit::Map100thMM));
    EditEngine aEngine(xPool.get());
    aEngine.SetRefDevice(pDevice.get());
    setUpTextEngine(aEngine, rParagraphs, sal_Int64(0x7fffffff) * 360, fFontPt, rFontName,
                    rFontOfParagraph, rLevelOfParagraph);
    return static_cast<sal_Int64>(aEngine.CalcTextWidth() * 360 * textWidthPartFor(rFontName));
}

// The width of the widest word of rParagraphs at fFontPt points, in EMU: the width a box has to
// have for the text to break between words only.
static sal_Int64 measureWidestWord(const std::vector<OUString>& rParagraphs, double fFontPt,
                                   std::u16string_view rFontName,
                                   const std::vector<double>& rFontOfParagraph = {},
                                   const std::vector<sal_Int32>& rLevelOfParagraph = {})
{
    // a line may break at a space and after a hyphen, so Twenty-one is two words here; a word
    // keeps the size and the level of its paragraph
    std::vector<OUString> aWords;
    std::vector<double> aFonts;
    std::vector<sal_Int32> aLevels;
    for (size_t nParagraph = 0; nParagraph < rParagraphs.size(); ++nParagraph)
    {
        const OUString& rParagraph(rParagraphs[nParagraph]);
        const auto aTake = [&](OUStringBuffer& rWord) {
            aWords.push_back(rWord.makeStringAndClear());
            aFonts.push_back(nParagraph < rFontOfParagraph.size() ? rFontOfParagraph[nParagraph]
                                                                  : fFontPt);
            aLevels.push_back(nParagraph < rLevelOfParagraph.size() ? rLevelOfParagraph[nParagraph]
                                                                    : 1);
        };
        OUStringBuffer aWord;
        for (sal_Int32 n = 0; n < rParagraph.getLength(); ++n)
        {
            const sal_Unicode c(rParagraph[n]);
            if (c == ' ')
            {
                if (!aWord.isEmpty())
                    aTake(aWord);
                continue;
            }
            aWord.append(c);
            if (c == '-')
                aTake(aWord);
        }
        if (!aWord.isEmpty())
            aTake(aWord);
    }
    return measureWidestLine(aWords, fFontPt, rFontName, aFonts, aLevels);
}

// How deep in the data the node the shape rShape shows stands: 1 for a node of the document, 2
// for a child of one, and so on; 0 where the shape shows no node.
// How deep in the data the node with the model id rNodeId stands: 1 for a node of the document,
// 2 for a child of one, and so on; 0 for the document itself or an id no connection leads to.
static sal_Int32 dataDepthOfNode(const SmartArtDiagram& rDgm, const OUString& rNodeId)
{
    if (!rDgm.getData())
        return 0;
    OUString aNode(rNodeId);
    sal_Int32 nDepth(0);
    for (int nStep = 0; nStep < 32 && !aNode.isEmpty(); ++nStep)
    {
        OUString aParent;
        for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             rDgm.getData()->getConnections())
            if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
                && rConnection->msDestId == aNode)
            {
                aParent = rConnection->msSourceId;
                break;
            }
        if (aParent.isEmpty())
            break;
        ++nDepth;
        aNode = aParent;
    }
    return nDepth;
}

// The font a paragraph of level nLevel is set in, in points, where the first level is at fFont:
// each level below is fSecondPart of the one above, rounded to whole points as the drawings
// carry them, 37 under 48 and 22 under 28.
static double fontOfLevel(double fFont, sal_Int32 nLevel, double fSecondPart)
{
    if (nLevel <= 1)
        return fFont;
    double fPart(1.0);
    for (sal_Int32 n = 2; n <= nLevel; ++n)
        fPart *= fSecondPart;
    return std::round(fFont * fPart);
}

// The part of the first level's font the deeper levels of the text node rNode, named rName, are
// set in: the factor of a "secFontSz refType primFontSz" stated for the node, by the nodes
// above in rConstraints or by the node itself, Bracket_List's desTx at the whole of it; 0.78
// where none is stated, as the drawings have the children's texts wherever two sizes stand, 27
// for 35 and 21 for 27.
static double secondFontPartOf(const std::vector<Constraint>& rConstraints, const LayoutNode& rNode,
                               std::u16string_view rName)
{
    std::optional<double> oPart;
    const auto aTake = [&oPart](const Constraint& rConstraint) {
        if (rConstraint.mnType != XML_secFontSz || rConstraint.mnRefType != XML_primFontSz)
            return;
        oPart = rConstraint.mfFactor > 0.0 ? rConstraint.mfFactor : 1.0;
    };
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.msForName == rName)
            aTake(rConstraint);
    std::vector<Constraint> aOwn;
    gatherOwnConstraints(rNode, aOwn);
    for (const Constraint& rConstraint : aOwn)
        if (rConstraint.msForName.isEmpty())
            aTake(rConstraint);
    return oPart.value_or(0.78);
}

static std::vector<OUString> textParagraphsOf(const SmartArtDiagram& rDgm, const ShapePtr& rShape);
namespace
{
void gatherOwnAlgorithms(const LayoutAtom& rAtom, std::vector<const AlgAtom*>& rOut);
bool ownRuleGrows(const LayoutAtom& rAtom, sal_Int32 nType);
bool ownRuleStatesFinite(const LayoutAtom& rAtom, sal_Int32 nType);
bool ownRuleFrees(const LayoutAtom& rAtom, sal_Int32 nType);
std::optional<std::pair<double, double>> ownRuleFactorOf(const LayoutAtom& rAtom, sal_Int32 nType);
}

// The space after the paragraphs of the text node rNode, from the parameters of its text
// algorithm in percent of the font, "lnSpAfParP" after a first-level paragraph and "lnSpAfChP"
// after a deeper one, each 0.35 and 0.15 where not stated.
static SpaceAfterParts spaceAfterPartsOf(const LayoutNode& rNode)
{
    SpaceAfterParts aParts(0.35, 0.15);
    std::vector<const AlgAtom*> aAlgorithms;
    gatherOwnAlgorithms(rNode, aAlgorithms);
    if (aAlgorithms.empty() || aAlgorithms.front()->getType() != XML_tx)
        return aParts;
    const AlgAtom::ParamMap& rParams(aAlgorithms.front()->getMap());
    if (const auto aParent = rParams.find(XML_lnSpAfParP); aParent != rParams.end())
        aParts.first = aParent->second / 100.0;
    if (const auto aChild = rParams.find(XML_lnSpAfChP); aChild != rParams.end())
        aParts.second = aChild->second / 100.0;
    return aParts;
}

// The font size the constraints rConstraints state for the node named rName, primFontSz, in
// points, followed through "refType primFontSz refForName X" to the node that states a value;
// nothing where none is stated.
static std::optional<double> statedFontFor(const std::vector<Constraint>& rConstraints,
                                           const OUString& rName)
{
    std::optional<double> oSize;
    OUString aFollow(rName);
    for (int nStep = 0; nStep < 4 && !oSize; ++nStep)
    {
        OUString aNext;
        for (const Constraint& rConstraint : rConstraints)
        {
            if (rConstraint.mnType != XML_primFontSz || rConstraint.msForName != aFollow)
                continue;
            if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 0.0)
                oSize = rConstraint.mfValue;
            else if (rConstraint.mnRefType == XML_primFontSz && !rConstraint.msRefForName.isEmpty())
                aNext = rConstraint.msRefForName;
        }
        if (oSize || aNext.isEmpty())
            break;
        aFollow = aNext;
    }
    return oSize;
}

// The group of text nodes the node named rName belongs to, by the name of the node that states
// the size, followed through "primFontSz refType primFontSz refForName X", and through a node's
// own "primFontSz refType secFontSz" on to "secFontSz refType primFontSz refForName X",
// Horizontal_Bullet_List's desTx taking parTx's size that way; the node itself where no link
// leads on, a group of its own. pPart, where given, is set to the part of the group's font the
// node is stated at, the factors of the primFontSz links multiplied, 1 where none states one.
static OUString fontGroupOf(const SmartArtDiagram& rDgm,
                            const std::vector<Constraint>& rConstraints, const OUString& rName,
                            double* pPart = nullptr)
{
    if (pPart)
        *pPart = 1.0;
    std::map<OUString, const LayoutNode*> aNodes;
    if (rDgm.getLayout() && rDgm.getLayout()->getNode())
        gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);
    OUString aFollow(rName);
    for (int nStep = 0; nStep < 6; ++nStep)
    {
        // the statements for the node, by the nodes above and by itself
        std::vector<Constraint> aFor;
        for (const Constraint& rConstraint : rConstraints)
            if (rConstraint.msForName == aFollow)
                aFor.push_back(rConstraint);
        const auto aNode = aNodes.find(aFollow);
        if (aNode != aNodes.end() && aNode->second)
        {
            std::vector<Constraint> aOwn;
            gatherOwnConstraints(*aNode->second, aOwn);
            for (const Constraint& rConstraint : aOwn)
                if (rConstraint.msForName.isEmpty())
                    aFor.push_back(rConstraint);
        }
        OUString aNext;
        double fLinkPart(1.0);
        bool bStated(false), bOverSecond(false), bEqual(false);
        for (const Constraint& rConstraint : aFor)
        {
            if (rConstraint.mnType != XML_primFontSz)
                continue;
            if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 0.0)
                bStated = true;
            else if (rConstraint.mnRefType == XML_primFontSz && !rConstraint.msRefForName.isEmpty())
            {
                aNext = rConstraint.msRefForName;
                fLinkPart = rConstraint.mfFactor > 0.0 ? rConstraint.mfFactor : 1.0;
                bEqual = rConstraint.mnOperator == XML_equ;
            }
            else if (rConstraint.mnRefType == XML_secFontSz)
                bOverSecond = true;
        }
        if (bOverSecond && aNext.isEmpty())
            for (const Constraint& rConstraint : aFor)
                if (rConstraint.mnType == XML_secFontSz && rConstraint.mnRefType == XML_primFontSz
                    && !rConstraint.msRefForName.isEmpty())
                    aNext = rConstraint.msRefForName;
        // an op equ link holds over a size the node states itself
        if ((bStated && !bEqual) || aNext.isEmpty())
            break;
        if (pPart)
            *pPart *= fLinkPart;
        aFollow = aNext;
    }
    // no link: all data nodes below a layout node that states one font for them without a name,
    // "primFontSz for ch ptType node op equ", are one group
    if (aFollow == rName)
        for (const Constraint& rConstraint : rConstraints)
            if (rConstraint.mnType == XML_primFontSz && rConstraint.mnOperator == XML_equ
                && rConstraint.msForName.isEmpty() && rConstraint.mnPointType == XML_node
                && (rConstraint.mnFor == XML_ch || rConstraint.mnFor == XML_des)
                && !rConstraint.msStatedBy.isEmpty())
                return "ptType node below " + rConstraint.msStatedBy;
    return aFollow;
}

// Whether the layout of rDgm states one font for every node named rName, "primFontSz for des
// forName rName op equ" on any of its layout nodes.
static bool statesOneFontFor(const SmartArtDiagram& rDgm, std::u16string_view rName)
{
    if (!rDgm.getLayout() || !rDgm.getLayout()->getNode())
        return false;
    bool bFound(false);
    std::function<void(const LayoutAtom&)> aWalk = [&](const LayoutAtom& rAtom) {
        for (const LayoutAtomPtr& pBelow : rAtom.getChildren())
        {
            if (bFound)
                return;
            if (const ConstraintAtom* pConstraint
                = dynamic_cast<const ConstraintAtom*>(pBelow.get()))
            {
                const Constraint& rOne(pConstraint->getConstraint());
                if (rOne.mnType == XML_primFontSz && rOne.mnOperator == XML_equ
                    && rOne.msForName == rName)
                    bFound = true;
            }
            else
                aWalk(*pBelow);
        }
    };
    aWalk(*rDgm.getLayout()->getNode());
    return bFound;
}

// The font the group of rName is laid out with on this pass, in points, the stated size on the
// first pass or 65 where none is stated.
static double fontOfGroup(const SmartArtDiagram& rDgm, const std::vector<Constraint>& rConstraints,
                          const OUString& rName, OUString& rGroup)
{
    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    double fPart(1.0);
    rGroup = fontGroupOf(rDgm, rConstraints, rName, &fPart);
    auto aCandidate = rMutable.getFontCandidates().find(rGroup);
    if (aCandidate == rMutable.getFontCandidates().end())
    {
        aCandidate = rMutable.getFontCandidates()
                         .emplace(rGroup, statedFontFor(rConstraints, rName).value_or(65.0))
                         .first;
    }
    // A node stated as a part of its group's font, "primFontSz forName txLvl2 refType
    // primFontSz refForName txLvl1 fact 0.78" on Vertical_Circle_List, is at that part, rounded
    // to whole points as the drawings carry it: 12 beside a txLvl1 of 16.
    if (fPart != 1.0)
        return std::round(aCandidate->second * fPart);
    return aCandidate->second;
}

// size in points the text of rShape was given by hand (custT), none where it follows the
// Diagram's font
static std::optional<double> ownTextSizeOf(const SmartArtDiagram& rDgm, const ShapePtr& rShape)
{
    const rtl::Reference<svx::diagram::Point> xData(firstDataNodeOf(rDgm, rShape));
    if (!xData.is() || !xData->mbCustomText || !rDgm.getData())
        return std::nullopt;
    sal_Int32 nParagraph(-1);
    const Reference<text::XText> xText(
        rDgm.getData()->getMasterXShapeForPoint(*xData, nParagraph), UNO_QUERY);
    const Reference<beans::XPropertySet> xStart(
        xText.is() ? xText->getStart() : Reference<text::XTextRange>(), UNO_QUERY);
    float fHeight(0.0);
    if (xStart.is() && (xStart->getPropertyValue(u"CharHeight"_ustr) >>= fHeight)
        && fHeight > 0.0)
        return fHeight;
    return std::nullopt;
}

// font size in points of the text of rShape: its group's font, or the size given by hand;
// a text with a size of its own takes no part in the group's fit and rGroup stays empty
static double textFontOf(const SmartArtDiagram& rDgm, const std::vector<Constraint>& rConstraints,
                         const ShapePtr& rShape, OUString& rGroup)
{
    if (const std::optional<double> oOwn = ownTextSizeOf(rDgm, rShape))
    {
        rGroup.clear();
        return *oOwn;
    }
    return fontOfGroup(rDgm, rConstraints, rShape->getInternalName(), rGroup);
}

// The part of the font the margins of the text node named rName add on one axis where the
// layout states none for a side: 0.56 of the font for each side of lMarg and rMarg, or of tMarg
// and bMarg with bOverUnder, that neither the nodes above in rConstraints nor the node itself in
// rOwn state in any way. Every layout of the corpus that states no margin has its drawing carry
// 0.56 of the font there, Word_Snake's sides included.
static double unstatedMarginPart(const std::vector<Constraint>& rConstraints,
                                 const std::vector<Constraint>& rOwn, std::u16string_view rName,
                                 bool bOverUnder)
{
    const sal_Int32 nFirst(bOverUnder ? XML_tMarg : XML_lMarg);
    const sal_Int32 nSecond(bOverUnder ? XML_bMarg : XML_rMarg);
    std::set<sal_Int32> aStated;
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.msForName == rName)
            aStated.insert(rConstraint.mnType);
    for (const Constraint& rConstraint : rOwn)
        if (rConstraint.msForName.isEmpty())
            aStated.insert(rConstraint.mnType);
    return (aStated.count(nFirst) ? 0.0 : 0.56) + (aStated.count(nSecond) ? 0.0 : 0.56);
}

namespace
{
// margin of a text node on one side: a part of its font, a length in EMU and, on the left
// and the right only, a part of its width
struct TextMargin
{
    double mfPartOfFont = 0.0;
    sal_Int64 mnLength = 0;
    double mfPartOfWidth = 0.0;
};
}

// the margin nSide of the text node rNode named rName, as stated by the nodes above or by the
// node itself; a side stated nowhere gets the default of unstatedMarginPart
static TextMargin textMarginOf(const SmartArtDiagram& rDgm,
                               const std::vector<Constraint>& rConstraints, const LayoutNode& rNode,
                               const rtl::Reference<svx::diagram::Point>& rPoint,
                               std::u16string_view rName, sal_Int32 nSide)
{
    TextMargin aMargin;
    const bool bOverUnder(nSide == XML_tMarg || nSide == XML_bMarg);
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.msForName != rName || rConstraint.mnType != nSide)
            continue;
        if (rConstraint.mnRefType == XML_primFontSz)
            aMargin.mfPartOfFont += rConstraint.mfFactor;
        else if (rConstraint.mnRefType == XML_w && !bOverUnder)
            aMargin.mfPartOfWidth
                = 1.0 - (1.0 - aMargin.mfPartOfWidth) * (1.0 - rConstraint.mfFactor);
        else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 0.0 && bOverUnder)
            aMargin.mnLength += static_cast<sal_Int64>(
                o3tl::convert(rConstraint.mfValue, o3tl::Length::mm, o3tl::Length::emu));
    }
    std::vector<Constraint> aOwn;
    gatherDecidedConstraints(rDgm, rNode, rPoint, aOwn);
    for (const Constraint& rConstraint : aOwn)
    {
        if (!rConstraint.msForName.isEmpty() || rConstraint.mnType != nSide)
            continue;
        if (rConstraint.mnRefType == XML_primFontSz)
            aMargin.mfPartOfFont += rConstraint.mfFactor;
        else if (rConstraint.mnRefType == XML_tMarg || rConstraint.mnRefType == XML_bMarg)
            for (const Constraint& rOther : aOwn)
                if (rOther.mnType == rConstraint.mnRefType && rOther.mnRefType == XML_primFontSz)
                {
                    aMargin.mfPartOfFont += rOther.mfFactor;
                    break;
                }
        break;
    }
    // unstatedMarginPart looks at both sides of the axis, so we state the other one
    Constraint aOtherSide{};
    aOtherSide.mnType = bOverUnder ? (nSide == XML_tMarg ? XML_bMarg : XML_tMarg)
                                   : (nSide == XML_lMarg ? XML_rMarg : XML_lMarg);
    aOwn.push_back(aOtherSide);
    aMargin.mfPartOfFont += unstatedMarginPart(rConstraints, aOwn, rName, bOverUnder);
    return aMargin;
}

// The level of each paragraph of the text the shape rShape shows, in the order of
// textParagraphsOf: 1 for a paragraph of the node the shape stands for, one more for each step
// down in the data from there, so the descendants a node shows beside itself are at 2 and the
// descendants a node shows in place of itself as well, Bracket_List's desTx. Counted from the
// shallowest node shown where the shape stands for no node.
static std::vector<sal_Int32> textParagraphLevelsOf(const SmartArtDiagram& rDgm,
                                                    const ShapePtr& rShape, size_t nParagraphs);

// The height a text node that grows with its text needs, in EMU: its text at the Diagram's font
// of this pass, the deeper level smaller, in the width nWidth less the side margins, and the
// margins over and under it, stated for the node by the nodes above in rConstraints or by the
// node itself as parts of the font. rbWordsFit turns false where the widest word is wider than
// the width. The font candidate of the Diagram is set from the stated size where none stands
// yet. Nothing where the shape is no text node that grows.
static std::optional<sal_Int64> textDrivenHeight(const SmartArtDiagram& rDgm,
                                                 const std::vector<Constraint>& rConstraints,
                                                 const LayoutNode& rNode,
                                                 const rtl::Reference<svx::diagram::Point>& rPoint,
                                                 const ShapePtr& rShape, sal_Int64 nWidth,
                                                 bool& rbWordsFit, bool bGrowingOnly = true)
{
    std::vector<const AlgAtom*> aAlgorithms;
    gatherOwnAlgorithms(rNode, aAlgorithms);
    if (aAlgorithms.empty() || aAlgorithms.front()->getType() != XML_tx
        || (bGrowingOnly && !ownRuleGrows(rNode, XML_h)))
        return std::nullopt;
    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    const double fPoint(12700.0);
    OUString aGroup;
    const double fFont(textFontOf(rDgm, rConstraints, rShape, aGroup));
    const std::vector<OUString> aParagraphs(textParagraphsOf(rDgm, rShape));
    // the levels of the paragraphs, the node the shape stands for being the first
    const std::vector<sal_Int32> aLevels(textParagraphLevelsOf(rDgm, rShape, aParagraphs.size()));
    const double fSecondPart(secondFontPartOf(rConstraints, rNode, rShape->getInternalName()));
    std::vector<double> aFontOfParagraph;
    double fTextFont(fFont);
    for (size_t n = 0; n < aParagraphs.size(); ++n)
    {
        const double fOne(fontOfLevel(fFont, aLevels[n], fSecondPart));
        aFontOfParagraph.push_back(fOne);
        if (n == 0)
            fTextFont = fOne;
    }
    double fOverUnder(0.0), fSides(0.0);
    sal_Int64 nFixed(0);
    for (const sal_Int32 nSide : { XML_lMarg, XML_tMarg, XML_rMarg, XML_bMarg })
    {
        const TextMargin aMargin(
            textMarginOf(rDgm, rConstraints, rNode, rPoint, rShape->getInternalName(), nSide));
        if (nSide == XML_lMarg || nSide == XML_rMarg)
        {
            fSides += aMargin.mfPartOfFont;
            nWidth -= static_cast<sal_Int64>(nWidth * aMargin.mfPartOfWidth);
        }
        else
        {
            fOverUnder += aMargin.mfPartOfFont;
            nFixed += aMargin.mnLength;
        }
    }
    nWidth -= static_cast<sal_Int64>(fSides * fFont * fPoint);
    // the words stand in the width the shape leaves its text
    sal_Int64 nInset(0);
    const sal_Int64 nText(aParagraphs.empty()
                              ? 0
                              : measureTextHeightInShape(
                                  rShape, aParagraphs, std::max<sal_Int64>(nWidth, 1),
                                  nFixed + static_cast<sal_Int64>(fOverUnder * fFont * fPoint),
                                  fTextFont, aFontOfParagraph, aLevels,
                                  spaceAfterPartsOf(rNode), rDgm.getTextFont(), &nInset));
    if (!aParagraphs.empty()
        && measureWidestWord(aParagraphs, fTextFont, rDgm.getTextFont(), aFontOfParagraph,
                             aLevels)
               > nWidth - 2 * nInset)
        rbWordsFit = false;
    // the size is the node's own only where the layout grows the node for its text and the
    // group came down to it from the stated size; a node of a fixed height, and one whose group
    // never had a text that did not fit, keeps the size the text itself carries
    if (ownRuleGrows(rNode, XML_h) && rMutable.getGroupsThatShrank().count(aGroup))
        rMutable.getFittedFontSizes()[rShape->getInternalName()] = fTextFont;
    if (!rbWordsFit)
        rMutable.getGroupsThatDidNotFit().insert(aGroup);
    return nText + nFixed + static_cast<sal_Int64>(fOverUnder * fFont * fPoint);
}

// The group of the text node rShape, for the row or composite that finds its text does not fit.
static OUString textGroupOf(const SmartArtDiagram& rDgm,
                            const std::vector<Constraint>& rConstraints, const ShapePtr& rShape)
{
    OUString aGroup;
    fontOfGroup(rDgm, rConstraints, rShape->getInternalName(), aGroup);
    return aGroup;
}

// The width a text node that grows with its text across, "rule type=w", needs, in EMU: its
// widest paragraph on one line at the Diagram's font of this pass, the deeper level smaller,
// and the margins left and right of it stated as parts of the font. Nothing where the shape is
// no text node that grows that way, or holds no text.
static std::optional<sal_Int64> textDrivenWidth(const SmartArtDiagram& rDgm,
                                                const std::vector<Constraint>& rConstraints,
                                                const LayoutNode& rNode,
                                                const rtl::Reference<svx::diagram::Point>& rPoint,
                                                const ShapePtr& rShape)
{
    std::vector<const AlgAtom*> aAlgorithms;
    gatherOwnAlgorithms(rNode, aAlgorithms);
    if (aAlgorithms.empty() || aAlgorithms.front()->getType() != XML_tx
        || !ownRuleFrees(rNode, XML_w))
        return std::nullopt;
    const std::vector<OUString> aParagraphs(textParagraphsOf(rDgm, rShape));
    if (aParagraphs.empty())
        return std::nullopt;
    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    const double fPoint(12700.0);
    OUString aGroup;
    const double fFont(textFontOf(rDgm, rConstraints, rShape, aGroup));
    const std::vector<sal_Int32> aLevels(textParagraphLevelsOf(rDgm, rShape, aParagraphs.size()));
    const double fSecondPart(secondFontPartOf(rConstraints, rNode, rShape->getInternalName()));
    const double fTextFont(fontOfLevel(fFont, aLevels.front(), fSecondPart));
    double fSides(0.0);
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.msForName == rShape->getInternalName()
            && (rConstraint.mnType == XML_lMarg || rConstraint.mnType == XML_rMarg)
            && rConstraint.mnRefType == XML_primFontSz)
            fSides += rConstraint.mfFactor;
    std::vector<Constraint> aOwn;
    gatherDecidedConstraints(rDgm, rNode, rPoint, aOwn);
    std::set<sal_Int32> aSeen;
    for (const Constraint& rConstraint : aOwn)
    {
        if (!rConstraint.msForName.isEmpty() || aSeen.count(rConstraint.mnType))
            continue;
        aSeen.insert(rConstraint.mnType);
        if ((rConstraint.mnType == XML_lMarg || rConstraint.mnType == XML_rMarg)
            && rConstraint.mnRefType == XML_primFontSz)
            fSides += rConstraint.mfFactor;
    }
    fSides += unstatedMarginPart(rConstraints, aOwn, rShape->getInternalName(), false);
    if (rMutable.getGroupsThatShrank().count(aGroup))
        rMutable.getFittedFontSizes()[rShape->getInternalName()] = fTextFont;
    std::vector<double> aFontOfParagraph;
    for (size_t n = 0; n < aParagraphs.size(); ++n)
        aFontOfParagraph.push_back(fontOfLevel(fFont, aLevels[n], fSecondPart));
    return measureWidestLine(aParagraphs, fTextFont, rDgm.getTextFont(), aFontOfParagraph,
                             aLevels)
           + static_cast<sal_Int64>(fSides * fFont * fPoint);
}

// The paragraphs of the text the shape rShape shows: the text body it carries where it has one,
// the import, and the text of the data point it presents otherwise, read off the drawing as it
// stands, which is where a layout run again finds it.
// The connections of the points the presentation point rPoint shows, in the order of the
// paragraphs they make, the destination order of the connection; the shown points as the
// Diagram keeps them stand in the order they were met in the data.
static std::vector<const svx::diagram::Connection*>
shownInParagraphOrder(const svx::diagram::Point& rPoint)
{
    std::vector<const svx::diagram::Connection*> aOut;
    for (const svx::diagram::Connection* pShown : rPoint.maShows)
        if (pShown && pShown->mpSourcePoint)
            aOut.push_back(pShown);
    std::stable_sort(aOut.begin(), aOut.end(),
                     [](const svx::diagram::Connection* pA, const svx::diagram::Connection* pB) {
                         return pA->mnDestOrder < pB->mnDestOrder;
                     });
    return aOut;
}

static std::vector<OUString> textParagraphsOf(const SmartArtDiagram& rDgm, const ShapePtr& rShape)
{
    std::vector<OUString> aOut;
    if (rShape->getTextBody() && !rShape->getTextBody()->isEmpty())
    {
        for (const auto& rParagraph : rShape->getTextBody()->getParagraphs())
        {
            OUStringBuffer aLine;
            for (const auto& rRun : rParagraph->getRuns())
                aLine.append(rRun->getText());
            aOut.push_back(aLine.makeStringAndClear());
        }
        return aOut;
    }
    if (!rDgm.getLayout() || !rDgm.getData())
        return aOut;
    const auto aAppend = [&aOut](const OUString& rText) {
        if (rText.isEmpty())
            return;
        sal_Int32 nIndex(0);
        do
            aOut.push_back(rText.getToken(0, '\n', nIndex));
        while (nIndex >= 0);
    };
    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
    {
        if (rEntry.second != rShape || !rEntry.first.is())
            continue;
        // a shape that shows several points, the children of a node as its paragraphs, holds
        // the text of each of them, one after the other; one that shows the descendants of a
        // node and finds none holds no text, Vertical_Box_List's third child text
        if (!rEntry.first->maShows.empty())
        {
            for (const svx::diagram::Connection* pShown : shownInParagraphOrder(*rEntry.first))
                aAppend(rDgm.getData()->getTextForPoint(*pShown->mpSourcePoint));
            if (!aOut.empty())
                break;
        }
        {
            std::map<OUString, const LayoutNode*> aNodes;
            gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);
            const auto aNode
                = aNodes.find(rEntry.first->getPresentation().msPresentationLayoutName);
            if (aNode != aNodes.end() && aNode->second)
            {
                bool bShowsItself(true);
                if (aNode->second->getPresentationOf())
                {
                    for (const sal_Int32 nAxis : aNode->second->getPresentationOf()->maAxis)
                    {
                        if (nAxis == XML_des || nAxis == XML_ch)
                            bShowsItself = false;
                        else if (nAxis == XML_self || nAxis == XML_desOrSelf)
                            bShowsItself = true;
                    }
                }
                if (!bShowsItself)
                    break;
            }
        }
        const OUString& rNode(rEntry.first->getPresentation().msPresentationAssociationId);
        const rtl::Reference<svx::diagram::Point> xNode(rDgm.getData()->getPointByModelID(rNode));
        if (xNode.is())
            aAppend(rDgm.getData()->getTextForPoint(*xNode));
        break;
    }
    return aOut;
}

// A snake of text nodes that grow with their text both ways from a stated size, "w val 30" and
// "h val 35" with "rule w INF" and "rule h INF" on Word_Snake's nodes. A node grows from its
// stated size in steps of a thirty-second of it until its text stands in it: as wide as its
// widest line with the side margins, as high as its text at that width with the margins over
// and under; a margin the layout does not state is 0.56 of the font. Word_Snake's boxes are
// 70, 88, 56 and 78 thirty-seconds of their 30 mm wide and 66 of their 35 mm high. Nodes stated
// as equally high, "h for ch ptType node op equ", take the highest. The nodes stand in rows
// that fill the width, the whole block in the middle of the room and every row starting at the
// block's edge the snake grows from; where the rows are higher than the room or a node wider
// than it, the font of the nodes is a point smaller on the next pass. False where the children
// are not all such nodes, and the snake is laid out as any other.
static bool layoutTextDrivenSnake(const SmartArtDiagram& rDgm, const AlgAtom& rAlg,
                                  const ShapePtr& rShape,
                                  const std::vector<Constraint>& rConstraints)
{
    if (rShape->getChildren().empty() || rShape->getSize().Width <= 0
        || rShape->getSize().Height <= 0 || !rDgm.getLayout() || !rDgm.getLayout()->getNode())
        return false;
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == XML_sp || rConstraint.mnType == XML_sibSp)
            return false;
    std::map<OUString, const LayoutNode*> aNodes;
    gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);
    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    const double fPoint(12700.0);

    struct Cell
    {
        ShapePtr mpShape;
        sal_Int64 mnWidth = 0;
        sal_Int64 mnHeight = 0;
    };
    std::vector<Cell> aCells;
    OUString aGroup;
    for (const ShapePtr& pChild : rShape->getChildren())
    {
        const auto aNode = aNodes.find(pChild->getInternalName());
        if (aNode == aNodes.end() || !aNode->second)
            return false;
        const LayoutNode& rNode(*aNode->second);
        std::vector<const AlgAtom*> aAlgorithms;
        gatherOwnAlgorithms(rNode, aAlgorithms);
        if (aAlgorithms.empty() || aAlgorithms.front()->getType() != XML_tx
            || !ownRuleGrows(rNode, XML_w) || !ownRuleGrows(rNode, XML_h))
            return false;
        // the stated size in millimetres, and the margins as parts of the font
        std::vector<Constraint> aOwn;
        gatherOwnConstraints(rNode, aOwn);
        std::optional<double> oWidthMm, oHeightMm;
        std::map<sal_Int32, double> aMargins{ { XML_lMarg, 0.56 },
                                              { XML_rMarg, 0.56 },
                                              { XML_tMarg, 0.56 },
                                              { XML_bMarg, 0.56 } };
        for (const Constraint& rConstraint : aOwn)
        {
            if (!rConstraint.msForName.isEmpty())
                continue;
            if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_none
                && rConstraint.mfValue > 0.0)
                oWidthMm = rConstraint.mfValue;
            else if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_none
                     && rConstraint.mfValue > 0.0)
                oHeightMm = rConstraint.mfValue;
            else if (aMargins.count(rConstraint.mnType)
                     && rConstraint.mnRefType == XML_primFontSz)
                aMargins[rConstraint.mnType] = rConstraint.mfFactor;
        }
        if (!oWidthMm || !oHeightMm)
            return false;
        const std::vector<OUString> aParagraphs(textParagraphsOf(rDgm, pChild));
        const double fFont(textFontOf(rDgm, rConstraints, pChild, aGroup));
        const std::vector<sal_Int32> aLevels(
            textParagraphLevelsOf(rDgm, pChild, aParagraphs.size()));
        const double fSecondPart(secondFontPartOf(rConstraints, rNode, pChild->getInternalName()));
        std::vector<double> aFontOfParagraph;
        for (size_t n = 0; n < aParagraphs.size(); ++n)
            aFontOfParagraph.push_back(fontOfLevel(fFont, aLevels[n], fSecondPart));
        const double fTextFont(aFontOfParagraph.empty() ? fFont : aFontOfParagraph.front());
        const sal_Int64 nSides(static_cast<sal_Int64>(
            (aMargins[XML_lMarg] + aMargins[XML_rMarg]) * fFont * fPoint));
        const sal_Int64 nOverUnder(static_cast<sal_Int64>(
            (aMargins[XML_tMarg] + aMargins[XML_bMarg]) * fFont * fPoint));
        // grow from the stated size by thirty-seconds until the need is met
        const auto aGrown = [](sal_Int64 nStated, sal_Int64 nNeed) {
            const sal_Int64 nStep(std::max<sal_Int64>(1, nStated / 32));
            if (nNeed <= nStated)
                return nStated;
            return nStated + ((nNeed - nStated + nStep - 1) / nStep) * nStep;
        };
        Cell aCell;
        aCell.mpShape = pChild;
        const sal_Int64 nStatedWidth(static_cast<sal_Int64>(
            o3tl::convert(*oWidthMm, o3tl::Length::mm, o3tl::Length::emu)));
        const sal_Int64 nStatedHeight(static_cast<sal_Int64>(
            o3tl::convert(*oHeightMm, o3tl::Length::mm, o3tl::Length::emu)));
        const sal_Int64 nLine(
            aParagraphs.empty()
                ? 0
                : measureWidestLine(aParagraphs, fTextFont, rDgm.getTextFont(),
                                    aFontOfParagraph, aLevels));
        aCell.mnWidth = aGrown(nStatedWidth, nLine + nSides);
        const sal_Int64 nText(
            aParagraphs.empty()
                ? 0
                : measureTextHeightInShape(pChild, aParagraphs,
                                           std::max<sal_Int64>(1, aCell.mnWidth - nSides),
                                           nOverUnder, fTextFont, aFontOfParagraph, aLevels,
                                           spaceAfterPartsOf(rNode), rDgm.getTextFont()));
        aCell.mnHeight = aGrown(nStatedHeight, nText + nOverUnder);
        rMutable.getFittedFontSizes()[pChild->getInternalName()] = fTextFont;
        aCells.push_back(aCell);
    }

    // nodes stated as equally high take the highest
    bool bEqualHeights(false);
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == XML_h && rConstraint.mnFor == XML_ch
            && rConstraint.mnPointType == XML_node && rConstraint.mnOperator == XML_equ)
            bEqualHeights = true;
    if (bEqualHeights)
    {
        sal_Int64 nHighest(0);
        for (const Cell& rCell : aCells)
            nHighest = std::max(nHighest, rCell.mnHeight);
        for (Cell& rCell : aCells)
            rCell.mnHeight = nHighest;
    }

    // the rows, each as full as the width allows
    const sal_Int64 nRoomWidth(rShape->getSize().Width);
    const sal_Int64 nRoomHeight(rShape->getSize().Height);
    std::vector<std::vector<size_t>> aRows(1);
    sal_Int64 nRowWidth(0);
    bool bFits(true);
    for (size_t n = 0; n < aCells.size(); ++n)
    {
        if (aCells[n].mnWidth > nRoomWidth)
            bFits = false;
        if (!aRows.back().empty() && nRowWidth + aCells[n].mnWidth > nRoomWidth)
        {
            aRows.emplace_back();
            nRowWidth = 0;
        }
        aRows.back().push_back(n);
        nRowWidth += aCells[n].mnWidth;
    }
    sal_Int64 nBlockWidth(0), nBlockHeight(0);
    std::vector<sal_Int64> aRowHeights;
    for (const std::vector<size_t>& rRow : aRows)
    {
        sal_Int64 nWidth(0), nHeight(0);
        for (const size_t n : rRow)
        {
            nWidth += aCells[n].mnWidth;
            nHeight = std::max(nHeight, aCells[n].mnHeight);
        }
        nBlockWidth = std::max(nBlockWidth, nWidth);
        nBlockHeight += nHeight;
        aRowHeights.push_back(nHeight);
    }
    if (nBlockHeight > nRoomHeight)
        bFits = false;
    if (!bFits && !aGroup.isEmpty())
        rMutable.getGroupsThatDidNotFit().insert(aGroup);

    // the block in the middle of the room, the rows from the edge the snake grows from
    const auto aDirection = rAlg.getMap().find(XML_grDir);
    const sal_Int32 nGrowDirection(aDirection != rAlg.getMap().end() ? aDirection->second
                                                                     : XML_tL);
    const bool bFromRight(nGrowDirection == XML_tR || nGrowDirection == XML_bR);
    const sal_Int64 nBlockLeft((nRoomWidth - nBlockWidth) / 2);
    sal_Int64 nTop((nRoomHeight - nBlockHeight) / 2);
    for (size_t nRow = 0; nRow < aRows.size(); ++nRow)
    {
        sal_Int64 nX(bFromRight ? nBlockLeft + nBlockWidth : nBlockLeft);
        for (const size_t n : aRows[nRow])
        {
            const Cell& rCell(aCells[n]);
            if (bFromRight)
                nX -= rCell.mnWidth;
            const awt::Size aSize(static_cast<sal_Int32>(rCell.mnWidth),
                                  static_cast<sal_Int32>(rCell.mnHeight));
            rCell.mpShape->setPosition(
                awt::Point(static_cast<sal_Int32>(nX), static_cast<sal_Int32>(nTop)));
            rCell.mpShape->setSize(aSize);
            rCell.mpShape->setChildSize(aSize);
            if (!bFromRight)
                nX += rCell.mnWidth;
        }
        nTop += aRowHeights[nRow];
    }
    return true;
}

static std::vector<sal_Int32> textParagraphLevelsOf(const SmartArtDiagram& rDgm,
                                                    const ShapePtr& rShape, size_t nParagraphs)
{
    std::vector<sal_Int32> aOut;
    sal_Int32 nBase(0);
    if (rShape->getTextBody() && !rShape->getTextBody()->isEmpty())
    {
        // the paragraphs' own levels, as the text was put together
        for (const auto& rParagraph : rShape->getTextBody()->getParagraphs())
            aOut.push_back(rParagraph->getProperties().getLevel());
    }
    else if (rDgm.getLayout() && rDgm.getData())
        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
        {
            if (rEntry.second != rShape || !rEntry.first.is())
                continue;
            nBase = dataDepthOfNode(rDgm,
                                    rEntry.first->getPresentation().msPresentationAssociationId);
            // The levels count from the node the shape stands for where its text algorithm
            // starts the bullets at a level, "stBulletLvl", Bracket_List's descendants at level
            // 1; without it they count from the shallowest node shown, Numbered_List's
            // descendants all at level 0 in the drawing.
            {
                std::map<OUString, const LayoutNode*> aNodes;
                gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);
                const auto aNode
                    = aNodes.find(rEntry.first->getPresentation().msPresentationLayoutName);
                if (aNode != aNodes.end() && aNode->second)
                {
                    std::vector<const AlgAtom*> aAlgorithms;
                    gatherOwnAlgorithms(*aNode->second, aAlgorithms);
                    if (!aAlgorithms.empty() && aAlgorithms.front()->getType() == XML_tx
                        && !aAlgorithms.front()->getMap().count(XML_stBulletLvl))
                        nBase = 0;
                }
            }
            for (const svx::diagram::Connection* pShown : shownInParagraphOrder(*rEntry.first))
            {
                const sal_Int32 nDepth(dataDepthOfNode(rDgm, pShown->mpSourcePoint->msModelId));
                // one entry per line of the node's text, as textParagraphsOf splits it
                const OUString aText(rDgm.getData()->getTextForPoint(*pShown->mpSourcePoint));
                sal_Int32 nIndex(0);
                if (!aText.isEmpty())
                    do
                    {
                        aText.getToken(0, '\n', nIndex);
                        aOut.push_back(nDepth);
                    } while (nIndex >= 0);
            }
            break;
        }
    std::optional<sal_Int32> oShallowest;
    for (const sal_Int32 nDepth : aOut)
        if (!oShallowest || nDepth < *oShallowest)
            oShallowest = nDepth;
    if (nBase <= 0)
        nBase = oShallowest.value_or(1);
    for (sal_Int32& rDepth : aOut)
        rDepth = std::max<sal_Int32>(rDepth - nBase + 1, 1);
    while (aOut.size() < nParagraphs)
        aOut.push_back(1);
    aOut.resize(nParagraphs);
    return aOut;
}

static void gatherLayoutNodes(const LayoutAtom& rAtom, std::map<OUString, const LayoutNode*>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get()))
            rOut[pNode->getName()] = pNode;
        gatherLayoutNodes(*pChild, rOut);
    }
}

// The bounds a layout node states for its children, "op=lte" and "op=gte": no more than, no less
// than. The parsing of the constraints keeps the equalities only, so these are read straight off
// the atoms, the way the layout read them for rPoint, a choose in between decided. The walk
// stops at a layout node of its own.
static void gatherDecidedBounds(const SmartArtDiagram& rDgm, const LayoutAtom& rAtom,
                                const rtl::Reference<svx::diagram::Point>& rPoint,
                                std::vector<Constraint>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const ConstraintAtom* pConstraint = dynamic_cast<const ConstraintAtom*>(pChild.get()))
        {
            const Constraint& rConstraint(pConstraint->getConstraint());
            if (rConstraint.mnOperator == XML_lte || rConstraint.mnOperator == XML_gte)
                rOut.push_back(rConstraint);
            continue;
        }
        if (dynamic_cast<const ChooseAtom*>(pChild.get()))
        {
            for (const LayoutAtomPtr& pBranch : pChild->getChildren())
            {
                const ConditionAtom* pCondition
                    = dynamic_cast<const ConditionAtom*>(pBranch.get());
                if (pCondition && pCondition->getDecision(rDgm, rPoint, OUString()))
                {
                    gatherDecidedBounds(rDgm, *pBranch, rPoint, rOut);
                    break;
                }
            }
            continue;
        }
        gatherDecidedBounds(rDgm, *pChild, rPoint, rOut);
    }
}

void CompositeAlg::layoutShapeChildren(const SmartArtDiagram& rDgm, AlgAtom& rAlg, const ShapePtr& rShape,
                                       const std::vector<Constraint>& rConstraints)
{
    LayoutPropertyMap aProperties;
    LayoutProperty& rParent = aProperties[u""_ustr];

    sal_Int32 nParentXOffset = 0;

    // Track min/max vertical positions, so we can center everything at the end, if needed.
    sal_Int32 nVertMin = std::numeric_limits<sal_Int32>::max();
    sal_Int32 nVertMax = 0;

    // A composite at the top of the layout that asks for a shape of its own, "ar", width to
    // height, lays its children out in the largest box of that shape the frame holds, standing
    // in the middle of it; its constraints are parts of that box. Circle_Process asks for 2.4437
    // with four nodes, and its ellipse accent, 0.2332 of the width and 0.5699 of the height, is
    // round in that box. A composite further down was given its cell by the row or the snake
    // above it, which made the cell that shape already, and the cell is its box; the picture
    // lists fill their cells. Without an ar the room itself is the box, and an ar of 1 below the
    // top keeps the width no wider than the height, as before.
    sal_Int32 nParentYOffset = 0;
    const bool bAtTheTop(rDgm.getLayout() && rDgm.getLayout()->getNode()
                         && rDgm.getLayout()->getNode().get() == &rAlg.getLayoutNode());
    // A composite whose box another composite gave it lays out in the largest box of its own ar
    // in it as well: the box is cut by the outer composite's constraints and has a shape of its
    // own. Phased_Process's middle composite of ar 1.792 gets 0.2728 of the width and 0.6567 of
    // the height of a composite of ar 2.8316, 1.176 wide to its height, and its two circles of
    // 0.555 of its width and 0.99456 of its height are round in the box of 1.792 in the middle
    // of that, as the drawing has them.
    bool bInAComposite(false);
    if (const LayoutNode* pParentNode = rAlg.getLayoutNode().getParentLayoutNode())
    {
        std::function<void(const LayoutAtom&)> aReadAlgorithms = [&](const LayoutAtom& rAtom) {
            for (const LayoutAtomPtr& pChild : rAtom.getChildren())
            {
                if (dynamic_cast<const LayoutNode*>(pChild.get()))
                    continue;
                if (const AlgAtom* pAlgorithm = dynamic_cast<const AlgAtom*>(pChild.get()))
                {
                    bInAComposite = pAlgorithm->getType() == XML_composite;
                    if (!bInAComposite)
                        return;
                    continue;
                }
                aReadAlgorithms(*pChild);
            }
        };
        aReadAlgorithms(*pParentNode);
    }
    if ((bAtTheTop || bInAComposite) && rAlg.getAspectRatio() > 0.0)
    {
        const double fAspect(rAlg.getAspectRatio());
        sal_Int32 nWidth(rShape->getSize().Width);
        sal_Int32 nHeight(static_cast<sal_Int32>(nWidth / fAspect));
        if (nHeight > rShape->getSize().Height)
        {
            nHeight = rShape->getSize().Height;
            nWidth = static_cast<sal_Int32>(nHeight * fAspect);
        }
        nParentXOffset = (rShape->getSize().Width - nWidth) / 2;
        nParentYOffset = (rShape->getSize().Height - nHeight) / 2;
        rParent[XML_w] = nWidth;
        rParent[XML_h] = nHeight;
        rParent[XML_l] = nParentXOffset;
        rParent[XML_t] = nParentYOffset;
        rParent[XML_r] = nParentXOffset + nWidth;
        rParent[XML_b] = nParentYOffset + nHeight;
    }
    else if (rAlg.getAspectRatio() != 1.0)
    {
        rParent[XML_w] = rShape->getSize().Width;
        rParent[XML_h] = rShape->getSize().Height;
        rParent[XML_l] = 0;
        rParent[XML_t] = 0;
        rParent[XML_r] = rShape->getSize().Width;
        rParent[XML_b] = rShape->getSize().Height;
    }
    else
    {
        // Shrink width to be only as large as height.
        rParent[XML_w] = std::min(rShape->getSize().Width, rShape->getSize().Height);
        rParent[XML_h] = rShape->getSize().Height;
        if (rParent[XML_w] < rShape->getSize().Width)
            nParentXOffset = (rShape->getSize().Width - rParent[XML_w]) / 2;
        rParent[XML_l] = nParentXOffset;
        rParent[XML_t] = 0;
        rParent[XML_r] = rShape->getSize().Width - rParent[XML_l];
        rParent[XML_b] = rShape->getSize().Height;
    }

    for (const auto& rConstr : rConstraints)
    {
        // Apply direct constraints for all layout nodes.
        applyConstraintToLayout(rDgm, rShape, rConstraints, rConstr, aProperties,
                                const_cast<SmartArtDiagram&>(rDgm).getUserVariables());
    }

    // The bounds the node states for its children, no more than and no less than, are read once
    // for the node's own point; the equalities give a child its size and the bounds cap it.
    std::vector<Constraint> aBounds;
    {
        rtl::Reference<svx::diagram::Point> xOwn;
        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
            if (rEntry.second == rShape)
            {
                xOwn = rEntry.first;
                break;
            }
        gatherDecidedBounds(rDgm, rAlg.getLayoutNode(), xOwn, aBounds);
    }

    // A child of the layout node that has no shape of its own, a spacer with nothing stated
    // for it, is as large as the composite's box: Balance's rows are 0.2 and 0.8 of the height
    // of dummyMaxCanvas, and that is the height of the composite.
    for (const LayoutAtomPtr& pAtom : rAlg.getLayoutNode().getChildren())
    {
        const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pAtom.get());
        if (!pNode)
            continue;
        const bool bHasShape(std::any_of(rShape->getChildren().begin(),
                                         rShape->getChildren().end(),
                                         [pNode](const ShapePtr& pChild) {
                                             return pChild->getInternalName() == pNode->getName();
                                         }));
        if (!bHasShape)
        {
            if (!aProperties.count(pNode->getName()))
                aProperties[pNode->getName()] = rParent;
            // A bound of one side against the other holds for it as well, and what is stated
            // as a part of it reads the side so bounded: Continuous_Arrow_Process's dummy is
            // at most 0.4 of its width high, 9031 of the 15052, and userP of 0.25 of that is
            // the padding of 2258 the drawing has after the last column.
            LayoutProperty& rOwn(aProperties[pNode->getName()]);
            for (const Constraint& rBound : aBounds)
            {
                if (rBound.msForName != pNode->getName()
                    || rBound.msRefForName != rBound.msForName || rBound.mfFactor <= 0.0
                    || (rBound.mnType != XML_w && rBound.mnType != XML_h)
                    || (rBound.mnRefType != XML_w && rBound.mnRefType != XML_h))
                    continue;
                const sal_Int32 nLimit(
                    static_cast<sal_Int32>(rOwn[rBound.mnRefType] * rBound.mfFactor));
                rOwn[rBound.mnType] = rBound.mnOperator == XML_lte
                                          ? std::min(rOwn[rBound.mnType], nLimit)
                                          : std::max(rOwn[rBound.mnType], nLimit);
            }
        }
    }

    for (auto& aCurrShape : rShape->getChildren())
    {
        // A child the algorithm above has placed already, a pyramid's level and accent, keeps
        // its place and size, and the others read that geometry where they refer to it.
        if (const_cast<SmartArtDiagram&>(rDgm).getLaidOutSideways().count(aCurrShape.get()))
        {
            LayoutProperty& rOwn = aProperties[aCurrShape->getInternalName()];
            const awt::Point aAt(aCurrShape->getPosition());
            const awt::Size aOwnSize(aCurrShape->getSize());
            rOwn[XML_l] = aAt.X;
            rOwn[XML_t] = aAt.Y;
            rOwn[XML_w] = aOwnSize.Width;
            rOwn[XML_h] = aOwnSize.Height;
            rOwn[XML_r] = aAt.X + aOwnSize.Width;
            rOwn[XML_b] = aAt.Y + aOwnSize.Height;
            rOwn[XML_ctrX] = aAt.X + aOwnSize.Width / 2;
            rOwn[XML_ctrY] = aAt.Y + aOwnSize.Height / 2;
            nVertMin = std::min(aAt.Y, nVertMin);
            nVertMax = std::max(aAt.Y + aOwnSize.Height, nVertMax);
            continue;
        }

        // Apply constraints from the current layout node for this child shape.
        // Previous child shapes may have changed aProperties.
        for (const auto& rConstr : rConstraints)
        {
            if (rConstr.msForName != aCurrShape->getInternalName())
            {
                continue;
            }

            applyConstraintToLayout(rDgm, rShape, rConstraints, rConstr, aProperties,
                                const_cast<SmartArtDiagram&>(rDgm).getUserVariables());
        }

        // A bound caps what the equalities gave: "w for ch forName=image refType=w op=lte
        // fact=0.33" keeps the image within a third of the width, "h refType=w refFor=ch
        // refForName=image op=lte" keeps it no taller than it is wide, and the two together, run
        // until nothing moves, make it a square of a third at most. What a bound refers to is
        // read where the equalities left it, the parent under the empty name.
        // A bound for ch that names no child is for every child, and one that holds a child's
        // side against its other side, "w for ch refType h refFor ch op gte fact 2", the child
        // at least twice as wide as high, cannot make the side grow past the box, so it is the
        // other side that gives: Circular_Picture_Callout's inner composite is 4064000 high in
        // a box of 8128000 by 5418667, half its width.
        for (size_t nPass = 0; nPass < 8; ++nPass)
        {
            bool bMoved(false);
            for (const Constraint& rBound : aBounds)
            {
                const bool bForEvery(rBound.msForName.isEmpty() && rBound.mnFor == XML_ch
                                     && rBound.mnPointType == XML_all);
                if (!bForEvery && rBound.msForName != aCurrShape->getInternalName())
                    continue;
                // a child nothing else was stated for has the box, and the bound works on that
                if (bForEvery && !aProperties.count(aCurrShape->getInternalName()))
                {
                    LayoutProperty& rOwn(aProperties[aCurrShape->getInternalName()]);
                    rOwn[XML_w] = rParent[XML_w];
                    rOwn[XML_h] = rParent[XML_h];
                }
                const auto aOwn = aProperties.find(aCurrShape->getInternalName());
                if (aOwn == aProperties.end() || !aOwn->second.count(rBound.mnType))
                    continue;
                const bool bAgainstItself(bForEvery ? rBound.mnRefFor == XML_ch
                                                        && rBound.msRefForName.isEmpty()
                                                    : rBound.msRefForName == rBound.msForName);
                std::optional<sal_Int32> oLimit;
                if (rBound.mnRefType == XML_none && rBound.mfValue != 0.0)
                    oLimit = static_cast<sal_Int32>(o3tl::convert(
                        rBound.mfValue, isFontUnit(rBound.mnType) ? o3tl::Length::pt
                                                                  : o3tl::Length::mm,
                        o3tl::Length::emu));
                else
                {
                    const auto aRef = aProperties.find(bAgainstItself ? aCurrShape->getInternalName()
                                                                      : rBound.msRefForName);
                    if (aRef != aProperties.end() && aRef->second.count(rBound.mnRefType))
                        oLimit = static_cast<sal_Int32>(aRef->second.at(rBound.mnRefType)
                                                        * rBound.mfFactor);
                }
                if (!oLimit)
                    continue;
                sal_Int32& rValue(aOwn->second[rBound.mnType]);
                const sal_Int32 nBefore(rValue);
                if (rBound.mnOperator == XML_lte)
                    rValue = std::min(rValue, *oLimit);
                else if (bAgainstItself && rBound.mfFactor > 0.0
                         && ((rBound.mnType == XML_w && rBound.mnRefType == XML_h)
                             || (rBound.mnType == XML_h && rBound.mnRefType == XML_w))
                         && *oLimit > (rBound.mnType == XML_w ? rParent[XML_w] : rParent[XML_h]))
                {
                    // the other side gives
                    sal_Int32& rOther(aOwn->second[rBound.mnRefType]);
                    const sal_Int32 nOtherBefore(rOther);
                    rOther = std::min<sal_Int32>(
                        rOther, static_cast<sal_Int32>(rValue / rBound.mfFactor));
                    bMoved = bMoved || rOther != nOtherBefore;
                }
                else
                    rValue = std::max(rValue, *oLimit);
                bMoved = bMoved || rValue != nBefore;
            }
            if (!bMoved)
                break;
        }

        // A child stated as high as it is wide, "h for ch forName imgShp refType w refFor ch
        // refForName imgShp op equ", stays so when a bound caps one of the two sides: the
        // other side follows, and the one with a bound on it is the one that stays. Vertical_
        // Picture_Accent_List's picture is 0.335 of the width, as high as wide and at most the
        // composite's height, a square of 3073 in the drawing where the capped height alone
        // left it 7564 wide.
        {
            const auto aOwn = aProperties.find(aCurrShape->getInternalName());
            for (const Constraint& rTie : rConstraints)
            {
                if (aOwn == aProperties.end() || rTie.mnFor != XML_ch
                    || rTie.msForName != aCurrShape->getInternalName()
                    || rTie.msRefForName != rTie.msForName || rTie.mnOperator != XML_equ
                    || rTie.mfFactor <= 0.0
                    || !((rTie.mnType == XML_h && rTie.mnRefType == XML_w)
                         || (rTie.mnType == XML_w && rTie.mnRefType == XML_h))
                    || !aOwn->second.count(XML_w) || !aOwn->second.count(XML_h))
                    continue;
                const sal_Int32 nWanted(
                    static_cast<sal_Int32>(aOwn->second.at(rTie.mnRefType) * rTie.mfFactor));
                if (std::abs(nWanted - aOwn->second.at(rTie.mnType)) <= 1)
                    continue;
                bool bTypeBound(false);
                bool bRefBound(false);
                for (const Constraint& rBound : aBounds)
                {
                    if (rBound.msForName != rTie.msForName || rBound.msRefForName == rTie.msForName)
                        continue;
                    if (rBound.mnType == rTie.mnType)
                        bTypeBound = true;
                    if (rBound.mnType == rTie.mnRefType)
                        bRefBound = true;
                }
                if (bTypeBound && !bRefBound)
                    aOwn->second[rTie.mnRefType] = static_cast<sal_Int32>(
                        aOwn->second.at(rTie.mnType) / rTie.mfFactor);
                else if (bRefBound && !bTypeBound)
                    aOwn->second[rTie.mnType] = nWanted;
            }
        }

        // the height a text child that grows asks for, where it does
        std::optional<sal_Int64> oTextNeed;
        // A text child that grows with its text, "rule type=h val=INF", is as high as its text
        // at the Diagram's font and its margins, and the bounds below cap it: Horizontal_
        // Bullet_List's child text of three lines is 5378 high under a heading of 0.4 of the
        // width, and the composite centres the two in its room. A text wider than its box or
        // higher than the composite tells the Diagram so, and the next pass is a point smaller.
        {
            std::map<OUString, const LayoutNode*> aLayoutNodes;
            gatherLayoutNodes(rAlg.getLayoutNode(), aLayoutNodes);
            const auto aChildNode = aLayoutNodes.find(aCurrShape->getInternalName());
            auto aOwn = aProperties.find(aCurrShape->getInternalName());
            // A shape turned by a quarter keeps its text upright, so the text runs along the
            // box's width as the layout states it, Numbered_List's descriptions on their side;
            // a shape turned otherwise is left as it is, and so is a turned one whose rules
            // grow its height in stated steps, Vertical_Chevron_List's chevrons.
            const sal_Int32 nTurn(
                (((aCurrShape->getRotation() + aCurrShape->getDiagramRotation()) / PER_DEGREE) % 180
                 + 180)
                % 180);
            const bool bTurned(aCurrShape->getRotation() != 0
                               || aCurrShape->getDiagramRotation() != 0);
            if (aChildNode != aLayoutNodes.end() && aChildNode->second
                && (nTurn == 0 || nTurn == 90)
                && (!bTurned || !ownRuleStatesFinite(*aChildNode->second, XML_h)))
            {
                rtl::Reference<svx::diagram::Point> xChildPoint;
                for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                    if (rEntry.second == aCurrShape)
                    {
                        xChildPoint = rEntry.first;
                        break;
                    }
                const sal_Int64 nWidth(aOwn != aProperties.end() && aOwn->second.count(XML_w)
                                           ? aOwn->second.at(XML_w)
                                           : rParent[XML_w]);
                bool bWordsFit(true);
                std::optional<sal_Int64> oNeed(textDrivenHeight(
                    rDgm, rConstraints, *aChildNode->second, xChildPoint, aCurrShape, nWidth,
                    bWordsFit));
                // Nodes of this name stated alike, "h for des forName desTx op equ", are as
                // high as the highest of them, so every composite centres the same content:
                // Horizontal_Bullet_List's child texts of two lines are as high as the one of
                // three, 5378, and the headings stand on one line at 3816.
                if (oNeed)
                {
                    bool bAlike(false);
                    for (const Constraint& rConstraint : rConstraints)
                        if (rConstraint.mnType == XML_h && rConstraint.mnOperator == XML_equ
                            && rConstraint.mnRefType == XML_none
                            && rConstraint.msRefForName.isEmpty()
                            && rConstraint.msForName == aCurrShape->getInternalName())
                            bAlike = true;
                    if (bAlike)
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                        {
                            if (!rEntry.second || rEntry.second == aCurrShape
                                || rEntry.second->getInternalName()
                                       != aCurrShape->getInternalName())
                                continue;
                            bool bOtherWordsFit(true);
                            const std::optional<sal_Int64> oOther(textDrivenHeight(
                                rDgm, rConstraints, *aChildNode->second, rEntry.first,
                                rEntry.second, nWidth, bOtherWordsFit));
                            if (oOther)
                                oNeed = std::max(*oNeed, *oOther);
                        }
                }
                if (oNeed)
                {
                    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
                    LayoutProperty& rOwn(aProperties[aCurrShape->getInternalName()]);
                    const sal_Int32 nTop(rOwn.count(XML_t) ? rOwn.at(XML_t) : 0);
                    // a text with a stated height grows from it in steps of a thirty-second of
                    // it; one stated as a part of a variable of the composite grows by the whole
                    // stated height. A text that needs less keeps the stated height
                    sal_Int64 nBox(*oNeed);
                    if (rOwn.count(XML_h) && rOwn.at(XML_h) > 0 && *oNeed > rOwn.at(XML_h))
                    {
                        const sal_Int64 nStated(rOwn.at(XML_h));
                        const bool bOfVariable(std::any_of(
                            rConstraints.begin(), rConstraints.end(),
                            [&aCurrShape](const Constraint& rConstraint) {
                                return rConstraint.mnType == XML_h
                                       && rConstraint.mnOperator == XML_none
                                       && isUserVariable(rConstraint.mnRefType)
                                       && rConstraint.msForName == aCurrShape->getInternalName();
                            }));
                        const sal_Int64 nStep(
                            std::max<sal_Int64>(1, bOfVariable ? nStated : nStated / 32));
                        oNeed = nStated + ((*oNeed - nStated + nStep - 1) / nStep) * nStep;
                        nBox = *oNeed;
                    }
                    else if (rOwn.count(XML_h) && rOwn.at(XML_h) > 0)
                        nBox = rOwn.at(XML_h);
                    if (nTop + *oNeed > rParent[XML_h])
                        rMutable.getGroupsThatDidNotFit().insert(
                            textGroupOf(rDgm, rConstraints, aCurrShape));
                    rOwn[XML_h] = static_cast<sal_Int32>(
                        std::min<sal_Int64>(nBox, std::max<sal_Int64>(1, rParent[XML_h] - nTop)));
                    if (rOwn.count(XML_b))
                        rOwn[XML_b] = nTop + rOwn[XML_h];
                    oTextNeed = *oNeed;
                    // the height the composite needs for this text, kept for a column above
                    // it to read on the next pass
                    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                        if (rEntry.second == rShape && rEntry.first.is())
                        {
                            sal_Int64& rKept(
                                rMutable.getNextContentHeights()[rEntry.first->msModelId]);
                            rKept = std::max<sal_Int64>(rKept, nTop + nBox);
                            break;
                        }
                }
            }
        }

        // A text child that grows across with its text, "rule type=w", is as wide as its widest
        // line and its side margins, within the width the composite gave it: Accented_Picture's
        // child texts are 4939 wide beside their circles, not the rest of the row, and Vertical_
        // Circle_List's composite shrinks to its texts.
        {
            std::map<OUString, const LayoutNode*> aLayoutNodes;
            gatherLayoutNodes(rAlg.getLayoutNode(), aLayoutNodes);
            const auto aChildNode = aLayoutNodes.find(aCurrShape->getInternalName());
            auto aOwn = aProperties.find(aCurrShape->getInternalName());
            if (aChildNode != aLayoutNodes.end() && aChildNode->second && aOwn != aProperties.end()
                && aCurrShape->getDiagramRotation() == 0 && aCurrShape->getRotation() == 0)
            {
                rtl::Reference<svx::diagram::Point> xChildPoint;
                for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                    if (rEntry.second == aCurrShape)
                    {
                        xChildPoint = rEntry.first;
                        break;
                    }
                const std::optional<sal_Int64> oNeed(textDrivenWidth(
                    rDgm, rConstraints, *aChildNode->second, xChildPoint, aCurrShape));
                if (oNeed)
                {
                    LayoutProperty& rOwn(aOwn->second);
                    const sal_Int32 nLeft(rOwn.count(XML_l) ? rOwn.at(XML_l) : 0);
                    const sal_Int64 nRoom(std::max<sal_Int64>(1, rParent[XML_w] - nLeft));
                    if (*oNeed > nRoom)
                        const_cast<SmartArtDiagram&>(rDgm).getGroupsThatDidNotFit().insert(
                            textGroupOf(rDgm, rConstraints, aCurrShape));
                    // In a composite the text is the width, within the width stated for it:
                    // Circular_Picture_Callout's texts beside their circles are as wide as
                    // their words, 0.11 of the frame off at the stated width.
                    const sal_Int64 nStated(rOwn.count(XML_w) ? rOwn.at(XML_w) : nRoom);
                    rOwn[XML_w] = static_cast<sal_Int32>(
                        std::max<sal_Int64>(
                            1, std::min<sal_Int64>(*oNeed, std::min(nStated, nRoom))));
                    if (rOwn.count(XML_r))
                        rOwn[XML_r] = nLeft + rOwn[XML_w];
                }
            }
        }

        // A text child whose height the constraints fix has to hold its text at the Diagram's
        // font, or the next pass is a point smaller: Accented_Picture's child text of three
        // paragraphs beside its circle is 105 points high, and holds them at 28 points and not
        // at 29, which is the drawing's 28.
        if (!oTextNeed)
        {
            std::map<OUString, const LayoutNode*> aLayoutNodes;
            gatherLayoutNodes(rAlg.getLayoutNode(), aLayoutNodes);
            const auto aChildNode = aLayoutNodes.find(aCurrShape->getInternalName());
            const auto aOwn = aProperties.find(aCurrShape->getInternalName());
            if (aChildNode != aLayoutNodes.end() && aChildNode->second
                && aOwn != aProperties.end() && aOwn->second.count(XML_h)
                && aCurrShape->getDiagramRotation() == 0 && aCurrShape->getRotation() == 0
                && !textParagraphsOf(rDgm, aCurrShape).empty())
            {
                rtl::Reference<svx::diagram::Point> xChildPoint;
                for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                    if (rEntry.second == aCurrShape)
                    {
                        xChildPoint = rEntry.first;
                        break;
                    }
                const sal_Int64 nWidth(aOwn->second.count(XML_w) ? aOwn->second.at(XML_w)
                                                                   : rParent[XML_w]);
                bool bWordsFit(true);
                const std::optional<sal_Int64> oNeed(textDrivenHeight(
                    rDgm, rConstraints, *aChildNode->second, xChildPoint, aCurrShape, nWidth,
                    bWordsFit, /*bGrowingOnly*/ false));
                if (oNeed && *oNeed > aOwn->second.at(XML_h))
                    const_cast<SmartArtDiagram&>(rDgm).getGroupsThatDidNotFit().insert(
                        textGroupOf(rDgm, rConstraints, aCurrShape));
            }
        }

        // A child may bound its own size as well, one side against the other: the parent text
        // of Basic_Chevron_Process is at most 0.4 of its width high, whatever the composite gave
        // it. The bound is read from the child's own constraints, the branch of a choose decided
        // by its point, and what it caps is what the children after it read.
        {
            std::map<OUString, const LayoutNode*> aLayoutNodes;
            gatherLayoutNodes(rAlg.getLayoutNode(), aLayoutNodes);
            const auto aChildNode = aLayoutNodes.find(aCurrShape->getInternalName());
            const auto aOwn = aProperties.find(aCurrShape->getInternalName());
            if (aChildNode != aLayoutNodes.end() && aChildNode->second
                && aOwn != aProperties.end())
            {
                rtl::Reference<svx::diagram::Point> xChildPoint;
                for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                    if (rEntry.second == aCurrShape)
                    {
                        xChildPoint = rEntry.first;
                        break;
                    }
                std::vector<Constraint> aOwnBounds;
                gatherDecidedBounds(rDgm, *aChildNode->second, xChildPoint, aOwnBounds);
                for (const Constraint& rBound : aOwnBounds)
                {
                    if (!rBound.msForName.isEmpty() || !rBound.msRefForName.isEmpty()
                        || (rBound.mnType != XML_w && rBound.mnType != XML_h)
                        || (rBound.mnRefType != XML_w && rBound.mnRefType != XML_h)
                        || rBound.mfFactor <= 0.0 || !aOwn->second.count(rBound.mnType)
                        || !aOwn->second.count(rBound.mnRefType))
                        continue;
                    const sal_Int32 nLimit(static_cast<sal_Int32>(
                        aOwn->second.at(rBound.mnRefType) * rBound.mfFactor));
                    sal_Int32& rSide(aOwn->second[rBound.mnType]);
                    const sal_Int32 nBefore(rSide);
                    rSide = rBound.mnOperator == XML_lte ? std::min(rSide, nLimit)
                                                         : std::max(rSide, nLimit);
                    if (rSide == nBefore)
                        continue;
                    // The composite gave the child its two edges as well, t and b for the
                    // height; the near edge stays and the far one follows the bounded side, so
                    // the text below the parent that stands at its b moves up with it.
                    const sal_Int32 nNear(rBound.mnType == XML_h ? XML_t : XML_l);
                    const sal_Int32 nFar(rBound.mnType == XML_h ? XML_b : XML_r);
                    if (aOwn->second.count(nNear) && aOwn->second.count(nFar))
                        aOwn->second[nFar] = aOwn->second.at(nNear) + rSide;
                    // A text that grows and is capped by its own bound below what it needs
                    // does not fit, and the Diagram's next pass is a point smaller: Horizontal_
                    // Bullet_List's heading of at most 0.4 of its width holds its line at 27
                    // points and not at 28, which is the drawing's 27.
                    if (rBound.mnType == XML_h && oTextNeed && rSide < *oTextNeed)
                        const_cast<SmartArtDiagram&>(rDgm).getGroupsThatDidNotFit().insert(
                            textGroupOf(rDgm, rConstraints, aCurrShape));
                }
            }
        }

        // Apply constraints from the child layout node for this child shape.
        // This builds on top of the own parent state + the state of previous shapes in the
        // same composite algorithm.
        const LayoutNode& rLayoutNode = rAlg.getLayoutNode();
        for (const auto& pDirectChild : rLayoutNode.getChildren())
        {
            auto pLayoutNode = dynamic_cast<LayoutNode*>(pDirectChild.get());
            if (!pLayoutNode)
            {
                continue;
            }

            if (pLayoutNode->getName() != aCurrShape->getInternalName())
            {
                continue;
            }

            for (const auto& pChild : pLayoutNode->getChildren())
            {
                auto pConstraintAtom = dynamic_cast<ConstraintAtom*>(pChild.get());
                if (!pConstraintAtom)
                {
                    continue;
                }

                const Constraint& rConstraint = pConstraintAtom->getConstraint();
                if (!rConstraint.msForName.isEmpty())
                {
                    continue;
                }

                if (!rConstraint.msRefForName.isEmpty())
                {
                    continue;
                }

                // What the child states for its own children or descendants is theirs and
                // says nothing about the child itself.
                if (rConstraint.mnFor != XML_self)
                {
                    continue;
                }

                // Either an absolute value or a factor of a property.
                if (rConstraint.mfValue == 0.0 && rConstraint.mnRefType == XML_none)
                {
                    continue;
                }

                // a bound, op lte or gte, caps or lifts what the equalities gave, and the step
                // for the child's own bounds above did that already
                if (rConstraint.mnOperator == XML_lte || rConstraint.mnOperator == XML_gte)
                {
                    continue;
                }

                Constraint aConstraint(rConstraint);
                aConstraint.msForName = pLayoutNode->getName();
                aConstraint.msRefForName = pLayoutNode->getName();

                applyConstraintToLayout(rDgm, rShape, rConstraints, aConstraint, aProperties,
                                    const_cast<SmartArtDiagram&>(rDgm).getUserVariables());
            }
        }

        // A child starts out as large as the composite's box, the box of its ar at the top,
        // and keeps that extent where its constraints say nothing about it: Balance's two rows
        // state their height and top only and are as wide as the square of ar 1, 5418667, the
        // parents 0.36 of that.
        awt::Size aSize(rParent[XML_w], rParent[XML_h]);
        awt::Point aPos(0, 0);

        const LayoutPropertyMap::const_iterator aPropIt
            = aProperties.find(aCurrShape->getInternalName());
        if (aPropIt != aProperties.end())
        {
            const LayoutProperty& rProp = aPropIt->second;
            LayoutProperty::const_iterator it, it2;

            // An edge or a middle can carry an offset of its own, lOff beside l and so on, that
            // moves it by that much from where the constraint put it.
            const auto aOffset = [&rProp](sal_Int32 nOff) {
                const auto aFound = rProp.find(nOff);
                return aFound == rProp.end() ? 0 : aFound->second;
            };

            // A size can carry an offset of its own as well, wOff beside w and hOff beside h:
            // Target_List's second circle is 0.75 of the first less a quarter of the space
            // between them, "hOff ... refType h refFor vertSpace2 fact -0.25", 0.7375 of it.
            if ((it = rProp.find(XML_w)) != rProp.end())
                aSize.Width = std::min(it->second + aOffset(XML_wOff), rShape->getSize().Width);
            if ((it = rProp.find(XML_h)) != rProp.end())
                aSize.Height = std::min(it->second + aOffset(XML_hOff), rShape->getSize().Height);

            if ((it = rProp.find(XML_l)) != rProp.end())
                aPos.X = it->second + aOffset(XML_lOff);
            else if ((it = rProp.find(XML_ctrX)) != rProp.end())
                aPos.X = it->second + aOffset(XML_ctrXOff) - aSize.Width / 2;
            else if ((it = rProp.find(XML_r)) != rProp.end())
                aPos.X = it->second + aOffset(XML_rOff) - aSize.Width;

            if ((it = rProp.find(XML_t)) != rProp.end())
                aPos.Y = it->second + aOffset(XML_tOff);
            else if ((it = rProp.find(XML_ctrY)) != rProp.end())
                aPos.Y = it->second + aOffset(XML_ctrYOff) - aSize.Height / 2;
            else if ((it = rProp.find(XML_b)) != rProp.end())
                aPos.Y = it->second + aOffset(XML_bOff) - aSize.Height;

            // Two edges say how large the shape is, each moved by its own offset: the
            // descendant box of Vertical_Chevron_List ends at the parent's b less half the
            // parent's width, its bOff.
            if ((it = rProp.find(XML_l)) != rProp.end() && (it2 = rProp.find(XML_r)) != rProp.end())
                aSize.Width = it2->second + aOffset(XML_rOff) - it->second - aOffset(XML_lOff);
            if ((it = rProp.find(XML_t)) != rProp.end() && (it2 = rProp.find(XML_b)) != rProp.end())
                aSize.Height = it2->second + aOffset(XML_bOff) - it->second - aOffset(XML_tOff);

            // A child placed by its middle stays inside the composite's box: the pictures of
            // Accented_Picture's pairs are centred at half the pair's width down, and the pair
            // is a strip as high as the pictures, so they stand at its bottom, which is its top.
            // A child larger than the box starts at its edge.
            if (rProp.count(XML_ctrX) && !rProp.count(XML_l) && !rProp.count(XML_r))
                aPos.X = std::max<sal_Int32>(
                    0, std::min<sal_Int32>(aPos.X, rParent[XML_w] - aSize.Width));
            if (rProp.count(XML_ctrY) && !rProp.count(XML_t) && !rProp.count(XML_b))
                aPos.Y = std::max<sal_Int32>(
                    0, std::min<sal_Int32>(aPos.Y, rParent[XML_h] - aSize.Height));

            aPos.X += nParentXOffset;
            aPos.Y += nParentYOffset;
            // A size the child states stands, also where it reaches past the composite's box:
            // Bending_Picture_Blocks's picture is 1.12 of the height wide from 0.3 of the width,
            // and the drawing has it end past the box. A side it does not state is the box's less
            // where the child starts: Picture_Lineup's text from half the height down is the lower
            // half.
            if (!rProp.count(XML_w) && !(rProp.count(XML_l) && rProp.count(XML_r)))
                aSize.Width = std::min(aSize.Width, rShape->getSize().Width - aPos.X);
            if (!rProp.count(XML_h) && !(rProp.count(XML_t) && rProp.count(XML_b)))
                aSize.Height = std::min(aSize.Height, rShape->getSize().Height - aPos.Y);

            // A connector that states its diam is an arc of a circle of that diameter, its band
            // as thick as its h, so its box is the diameter and the thickness. Its l and t, or
            // ctrX and ctrY, say where the middle of the circle stands, not a corner; where it
            // states none, the circle is about the shape its diam refers to, the gear a
            // connector of 1.1 of the gear's width runs round. Segmented_Cycle's arrow wedges run
            // round its wedges on a circle 0.84 of the composite across, from the middle of it.
            // A diam of less than nothing runs the circle the other way round, Gear's second
            // connector of -1.1 of its gear, and is as large as its size says.
            if ((it = rProp.find(XML_diam)) != rProp.end() && it->second != 0
                && aCurrShape->getSubType() == XML_conn)
            {
                const sal_Int32 nDiameter(std::abs(it->second));
                const sal_Int32 nThick(rProp.count(XML_h) ? rProp.at(XML_h) : 0);
                const sal_Int32 nBox(nDiameter + nThick);
                sal_Int32 nMiddleX(rShape->getSize().Width / 2);
                sal_Int32 nMiddleY(rShape->getSize().Height / 2);
                bool bMiddleStated(false);
                if ((it2 = rProp.find(XML_l)) != rProp.end())
                {
                    nMiddleX = it2->second + aOffset(XML_lOff) + nParentXOffset;
                    bMiddleStated = true;
                }
                else if ((it2 = rProp.find(XML_ctrX)) != rProp.end())
                {
                    nMiddleX = it2->second + aOffset(XML_ctrXOff) + nParentXOffset;
                    bMiddleStated = true;
                }
                if ((it2 = rProp.find(XML_t)) != rProp.end())
                    nMiddleY = it2->second + aOffset(XML_tOff) + nParentYOffset;
                else if ((it2 = rProp.find(XML_ctrY)) != rProp.end())
                    nMiddleY = it2->second + aOffset(XML_ctrYOff) + nParentYOffset;
                if (!bMiddleStated)
                    for (const Constraint& rConstraint : rConstraints)
                    {
                        if (rConstraint.mnType != XML_diam
                            || rConstraint.msForName != aCurrShape->getInternalName()
                            || rConstraint.msRefForName.isEmpty())
                            continue;
                        const auto aAbout = aProperties.find(rConstraint.msRefForName);
                        if (aAbout == aProperties.end())
                            continue;
                        const LayoutProperty& rAbout(aAbout->second);
                        const sal_Int32 nW(rAbout.count(XML_w) ? rAbout.at(XML_w) : 0);
                        const sal_Int32 nH(rAbout.count(XML_h) ? rAbout.at(XML_h) : 0);
                        const sal_Int32 nX(rAbout.count(XML_l)      ? rAbout.at(XML_l)
                                           : rAbout.count(XML_ctrX) ? rAbout.at(XML_ctrX) - nW / 2
                                           : rAbout.count(XML_r)    ? rAbout.at(XML_r) - nW
                                                                    : 0);
                        const sal_Int32 nY(rAbout.count(XML_t)      ? rAbout.at(XML_t)
                                           : rAbout.count(XML_ctrY) ? rAbout.at(XML_ctrY) - nH / 2
                                           : rAbout.count(XML_b)    ? rAbout.at(XML_b) - nH
                                                                    : 0);
                        nMiddleX = nX + nW / 2 + nParentXOffset;
                        nMiddleY = nY + nH / 2 + nParentYOffset;
                        break;
                    }
                aSize = awt::Size(nBox, nBox);
                aPos = awt::Point(nMiddleX - nBox / 2, nMiddleY - nBox / 2);
            }
        }
        else
        {
            // A child the constraints say nothing about takes the composite's box, which at
            // the top is the box of the composite's ar and not the whole room: Radial_Cluster's
            // ring stands in the square its composite of ar 1 makes, and its middle is 0.3 of
            // that square.
            aSize = awt::Size(rParent[XML_w], rParent[XML_h]);
            aPos = awt::Point(nParentXOffset, nParentYOffset);
        }

        aCurrShape->setSize(aSize);
        aCurrShape->setChildSize(aSize);
        aCurrShape->setPosition(aPos);

        // What the constraints left unsaid about the child is read off where it stands now by
        // the children after it: Stacked_Venn's second circle has "b refType b refFor ch
        // refForName comp1", and comp1 states no top of its own, it stands at the top of the
        // box, so its bottom is its height and nothing the constraints could have told. What
        // the constraints did say stays as said, a middle the box kept inside it included;
        // Basic_Target's rings follow the stated middle of the first.
        {
            LayoutProperty& rPlaced = aProperties[aCurrShape->getInternalName()];
            const auto aFill = [&rPlaced](sal_Int32 nWhat, sal_Int32 nValue) {
                if (!rPlaced.count(nWhat))
                    rPlaced[nWhat] = nValue;
            };
            // the properties are in the box's own coordinates, the offset of the box in the
            // room is added when a child is placed
            const sal_Int32 nLeft(aPos.X - nParentXOffset);
            const sal_Int32 nTop(aPos.Y - nParentYOffset);
            aFill(XML_l, nLeft);
            aFill(XML_t, nTop);
            aFill(XML_w, aSize.Width);
            aFill(XML_h, aSize.Height);
            aFill(XML_r, nLeft + aSize.Width);
            aFill(XML_b, nTop + aSize.Height);
            aFill(XML_ctrX, nLeft + aSize.Width / 2);
            aFill(XML_ctrY, nTop + aSize.Height / 2);
        }

        nVertMin = std::min(aPos.Y, nVertMin);
        nVertMax = std::max(aPos.Y + aSize.Height, nVertMax);

        NamedShapePairs& rDiagramFontHeights
            = const_cast<SmartArtDiagram&>(rDgm).getDiagramFontHeights();
        auto it = rDiagramFontHeights.find(aCurrShape->getInternalName());
        if (it != rDiagramFontHeights.end())
        {
            // Internal name matches: put drawingml::Shape to the relevant group, for
            // synchronized font height handling.
            it->second.insert({ aCurrShape, {} });
        }
    }

    // Children whose heights the constraints leave to the text, a heading of at most 0.4 of
    // its width over a list, stand in the middle of the room the composite has. Children that
    // are placed as parts of the composite's own height stand where the constraints put them,
    // also where they leave room: the last step of Step_Up_Process has no triangle at its top,
    // and its shapes stand at the same height in their box as in every other step. A box of
    // the composite's own shape stands in the middle of the room already.
    if (nParentYOffset > 0)
        return;
    if (!(nVertMin >= 0 && nVertMin <= nVertMax && nVertMax <= rParent[XML_h]))
        return;
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnFor != XML_ch || rConstraint.mnRefType != XML_h
            || !rConstraint.msRefForName.isEmpty() || rConstraint.mnRefFor == XML_ch)
            continue;
        if (rConstraint.mnType != XML_t && rConstraint.mnType != XML_b
            && rConstraint.mnType != XML_ctrY && rConstraint.mnType != XML_h)
            continue;
        const bool bOfAChild(std::any_of(
            rShape->getChildren().begin(), rShape->getChildren().end(),
            [&rConstraint](const ShapePtr& pChild) {
                return pChild->getInternalName() == rConstraint.msForName;
            }));
        if (bOfAChild)
            return;
    }

    // In a row that states where its nodes stand across, nodeVertAlign, the content stands
    // there in the composite's box too: List_Dots' composites, each as high as the row and
    // their texts of different heights, have their dots at the top in one line, as nodeVertAlign
    // t says, where the middle put every column at a height of its own.
    sal_Int32 nRowAlign(XML_mid);
    if (const LayoutNode* pParentNode = rAlg.getLayoutNode().getParentLayoutNode())
    {
        std::function<void(const LayoutAtom&)> aReadRow = [&](const LayoutAtom& rAtom) {
            for (const LayoutAtomPtr& pChild : rAtom.getChildren())
            {
                if (dynamic_cast<const LayoutNode*>(pChild.get()))
                    continue;
                if (const AlgAtom* pAlgorithm = dynamic_cast<const AlgAtom*>(pChild.get()))
                {
                    const AlgAtom::ParamMap& rRowMap(pAlgorithm->getMap());
                    const sal_Int32 nLinDir(rRowMap.count(XML_linDir)
                                                ? rRowMap.find(XML_linDir)->second
                                                : XML_fromL);
                    if (pAlgorithm->getType() == XML_lin
                        && (nLinDir == XML_fromL || nLinDir == XML_fromR)
                        && rRowMap.count(XML_nodeVertAlign))
                        nRowAlign = rRowMap.find(XML_nodeVertAlign)->second;
                    continue;
                }
                aReadRow(*pChild);
            }
        };
        aReadRow(*pParentNode);
    }

    sal_Int32 nDiff = rParent[XML_h] - (nVertMax - nVertMin);
    if (nDiff > 0)
    {
        const sal_Int32 nShift(nRowAlign == XML_t ? 0 : nRowAlign == XML_b ? nDiff : nDiff / 2);
        for (auto& aCurrShape : rShape->getChildren())
        {
            awt::Point aPosition = aCurrShape->getPosition();
            aPosition.Y += nShift;
            aCurrShape->setPosition(aPosition);
        }
    }
}

namespace
{
/**
 * The whole numbers an attribute holds, one per step of an axis, written apart by blanks.
 */
std::vector<sal_Int32> getNumberList(const AttributeList& rAttribs, sal_Int32 nToken)
{
    std::vector<sal_Int32> aValues;
    if (const std::string_view aValue = rAttribs.getView(nToken); !aValue.empty())
        for (size_t nIndex = 0; nIndex != std::string_view::npos;)
            aValues.push_back(o3tl::toInt32(o3tl::getToken(aValue, ' ', nIndex)));

    return aValues;
}
}

IteratorAttr::IteratorAttr( )
    : mnCnt( -1 )
    , mbHideLastTrans( true )
    , mnPtType( XML_all )
    , mnSt( 0 )
    , mnStep( 1 )
{
}

void IteratorAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    AttributeList attr( xAttr );
    maAxis = attr.getTokenList(XML_axis);
    maStart = getNumberList(attr, XML_st);
    maCount = getNumberList(attr, XML_cnt);
    mnCnt = attr.getInteger( XML_cnt, -1 );
    mbHideLastTrans = attr.getBool( XML_hideLastTrans, true );
    mnSt = attr.getInteger( XML_st, 0 );
    mnStep = attr.getInteger( XML_step, 1 );

    maPtType = attr.getTokenList(XML_ptType);
    mnPtType = maPtType.empty() ? XML_all : maPtType.front();
}

ConditionAttr::ConditionAttr()
    : mnFunc( 0 )
    , mnArg( 0 )
    , mnOp( 0 )
    , mnVal( 0 )
{
}

void ConditionAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    mnFunc = xAttr->getOptionalValueToken( XML_func, 0 );
    mnArg = xAttr->getOptionalValueToken( XML_arg, XML_none );
    mnOp = xAttr->getOptionalValueToken( XML_op, 0 );
    msVal = xAttr->getOptionalValue( XML_val );
    mnVal = xAttr->getOptionalValueToken( XML_val, 0 );
}

void LayoutAtom::dump(int level)
{
    SAL_INFO("oox.drawingml",  "level = " << level << " - " << msName << " of type " << typeid(*this).name() );
    for (const auto& pAtom : getChildren())
        pAtom->dump(level + 1);
}

ForEachAtom::ForEachAtom(LayoutNode& rLayoutNode, const Reference< XFastAttributeList >& xAttributes) :
    LayoutAtom(rLayoutNode)
{
    maIter.loadFromXAttr(xAttributes);
}

void ForEachAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

LayoutAtomPtr ForEachAtom::getRefAtom(const SmartArtDiagram& rDgm)
{
    if (!msRef.isEmpty())
    {
        const LayoutAtomMap& rLayoutAtomMap = rDgm.getLayout()->getLayoutAtomMap();
        LayoutAtomMap::const_iterator pRefAtom = rLayoutAtomMap.find(msRef);
        if (pRefAtom != rLayoutAtomMap.end())
            return pRefAtom->second;
        else
            SAL_WARN("oox.drawingml", "ForEach reference \"" << msRef << "\" not found");
    }
    return LayoutAtomPtr();
}

void ChooseAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

ConditionAtom::ConditionAtom(LayoutNode& rLayoutNode, bool isElse, const Reference< XFastAttributeList >& xAttributes) :
    LayoutAtom(rLayoutNode),
    mIsElse(isElse)
{
    maIter.loadFromXAttr( xAttributes );
    maCond.loadFromXAttr( xAttributes );
}

bool ConditionAtom::compareResult(sal_Int32 nOperator, sal_Int32 nFirst, sal_Int32 nSecond)
{
    switch (nOperator)
    {
    case XML_equ: return nFirst == nSecond;
    case XML_gt:  return nFirst >  nSecond;
    case XML_gte: return nFirst >= nSecond;
    case XML_lt:  return nFirst <  nSecond;
    case XML_lte: return nFirst <= nSecond;
    case XML_neq: return nFirst != nSecond;
    default:
        SAL_WARN("oox.drawingml", "unsupported operator: " << nOperator);
        return false;
    }
}

namespace
{
/**
 * Takes the connection list from rLayoutNode, navigates from rFrom on an edge
 * of type nType, using a direction determined by bSourceToDestination.
 */
OUString navigate(const SmartArtDiagram& rDgm, svx::diagram::TypeConstant nType, std::u16string_view rFrom,
                  bool bSourceToDestination)
{
    for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             rDgm.getData()->getConnections())
    {
        if (rConnection->mnXMLType != nType)
            continue;

        if (bSourceToDestination)
        {
            if (rConnection->msSourceId == rFrom)
                return rConnection->msDestId;
        }
        else
        {
            if (rConnection->msDestId == rFrom)
                return rConnection->msSourceId;
        }
    }

    return OUString();
}

/**
 * The parent of rFrom in the data tree, and the place rFrom takes among its children. A
 * transition Point hangs off the same parOf as the child it belongs to and takes that child's
 * place: Continuous_Cycle's arrow asks "axis par ch ptType doc node" on its sibTrans Point for
 * the count of the nodes, which is the parent's children.
 */
OUString parentOf(const SmartArtDiagram& rDgm, std::u16string_view rFrom, sal_Int32& rOrder)
{
    for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             rDgm.getData()->getConnections())
        if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
            && (rConnection->msDestId == rFrom || rConnection->msSibTransId == rFrom
                || rConnection->msParTransId == rFrom))
        {
            rOrder = rConnection->mnSourceOrder;
            return rConnection->msSourceId;
        }

    rOrder = 0;
    return OUString();
}

/**
 * The Points that hang off rFrom, in the order the file puts them in.
 */
std::vector<OUString> childrenOf(const SmartArtDiagram& rDgm, std::u16string_view rFrom,
                                 sal_Int32 nWanted)
{
    // A parOf holds three Points of a child: the child itself, the parTrans that draws what
    // joins it to its parent and the sibTrans that draws what separates it from the next one.
    // Which of the three a step reaches is the kind it asks for.
    std::vector<std::pair<sal_Int32, OUString>> aFound;
    for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             rDgm.getData()->getConnections())
    {
        if (rConnection->mnXMLType != svx::diagram::TypeConstant::XML_parOf
            || rConnection->msSourceId != rFrom)
            continue;

        if (nWanted == XML_parTrans)
            aFound.emplace_back(rConnection->mnSourceOrder, rConnection->msParTransId);
        else if (nWanted == XML_sibTrans)
            aFound.emplace_back(rConnection->mnSourceOrder, rConnection->msSibTransId);
        else
            aFound.emplace_back(rConnection->mnSourceOrder, rConnection->msDestId);
    }

    std::sort(aFound.begin(), aFound.end());

    std::vector<OUString> aOut;
    aOut.reserve(aFound.size());
    for (const auto& rFound : aFound)
        if (!rFound.second.isEmpty())
            aOut.push_back(rFound.second);

    return aOut;
}

/**
 * Everything that hangs below rFrom, at any depth, the nearest first. The walk keeps what it has
 * seen, a data tree that closes a ring would otherwise never end.
 */
void gatherDescendants(const SmartArtDiagram& rDgm, const OUString& rFrom,
                       std::set<OUString>& rSeen, std::vector<OUString>& rOut)
{
    if (!rSeen.insert(rFrom).second)
        return;

    for (const OUString& rChild : childrenOf(rDgm, rFrom, XML_all))
    {
        rOut.push_back(rChild);
        gatherDescendants(rDgm, rChild, rSeen, rOut);
    }
}

/**
 * The kind the Point with rId is of, XML_none where there is no such Point.
 */
sal_Int32 typeOfPoint(const SmartArtDiagram& rDgm, std::u16string_view rId)
{
    for (const rtl::Reference<svx::diagram::Point>& rPoint : rDgm.getData()->getPoints())
        if (rPoint.is() && rPoint->msModelId == rId)
            return static_cast<sal_Int32>(rPoint->mnXMLType);

    return XML_none;
}

/**
 * Whether a Point of type nType is one of the kind nWanted. A kind is not always a type: node
 * covers an assistant as well as an ordinary one, norm is the ordinary one alone, asst the
 * assistant alone, nonAsst everything that is not an assistant, and all every kind there is.
 */
bool isPointOfKind(sal_Int32 nType, sal_Int32 nWanted)
{
    switch (nWanted)
    {
        case XML_all:
        case XML_none:
            return true;
        case XML_node:
            return nType == XML_node || nType == XML_asst;
        case XML_norm:
            return nType == XML_node;
        case XML_asst:
            return nType == XML_asst;
        case XML_nonAsst:
            return nType != XML_asst;
        case XML_nonNorm:
            return nType != XML_node;
        default:
            return nType == nWanted;
    }
}

/**
 * The Points one step of an axis reaches from rFrom, in the order the file puts them in.
 */
std::vector<OUString> stepOfAxis(const SmartArtDiagram& rDgm, const OUString& rFrom,
                                 sal_Int32 nAxis, sal_Int32 nWanted)
{
    std::vector<OUString> aOut;

    switch (nAxis)
    {
        case XML_self:
            aOut.push_back(rFrom);
            break;

        case XML_root:
            for (const rtl::Reference<svx::diagram::Point>& rPoint : rDgm.getData()->getPoints())
                if (rPoint.is() && rPoint->mnXMLType == svx::diagram::TypeConstant::XML_doc)
                {
                    aOut.push_back(rPoint->msModelId);
                    break;
                }
            break;

        case XML_ch:
            aOut = childrenOf(rDgm, rFrom, nWanted);
            break;

        case XML_par:
        {
            sal_Int32 nOrder(0);
            const OUString aParent(parentOf(rDgm, rFrom, nOrder));
            if (!aParent.isEmpty())
                aOut.push_back(aParent);
            break;
        }

        // The follow and preced axes reach what stands after and before the node in the order
        // of the document, the siblings to that side with what hangs below them; here they
        // reach the siblings and their transitions like followSib and precedSib do, the
        // descendants of those siblings are not walked. Descending_Process's dots stand on
        // "axis follow ptType sibTrans cnt 1", the transition after the node, and were never
        // made, the axis reaching nothing.
        case XML_follow:
        case XML_preced:
        case XML_followSib:
        case XML_precedSib:
        {
            sal_Int32 nOrder(0);
            const OUString aParent(parentOf(rDgm, rFrom, nOrder));
            if (aParent.isEmpty())
                break;

            // A transition Point takes the place of the child it belongs to, so a step to
            // either side reaches the one of my own place as well as the ones beyond it.
            const bool bTransition(nWanted == XML_parTrans || nWanted == XML_sibTrans);
            std::vector<std::pair<sal_Int32, OUString>> aFound;
            for (const rtl::Reference<svx::diagram::Connection>& rConnection :
                 rDgm.getData()->getConnections())
            {
                if (rConnection->mnXMLType != svx::diagram::TypeConstant::XML_parOf
                    || rConnection->msSourceId != aParent)
                    continue;

                const sal_Int32 nSiblingOrder(rConnection->mnSourceOrder);
                const bool bKeep(nAxis == XML_followSib || nAxis == XML_follow
                                     ? (bTransition ? nSiblingOrder >= nOrder
                                                    : nSiblingOrder > nOrder)
                                     : (bTransition ? nSiblingOrder <= nOrder
                                                    : nSiblingOrder < nOrder));
                if (!bKeep)
                    continue;

                // The connection holds the child and both of its transitions. The step reaches
                // the one of the kind it asks for, and a sibling with no such id is passed over.
                const OUString aReached(nWanted == XML_parTrans   ? rConnection->msParTransId
                                        : nWanted == XML_sibTrans ? rConnection->msSibTransId
                                                                  : rConnection->msDestId);
                if (!aReached.isEmpty())
                    aFound.emplace_back(nSiblingOrder, aReached);
            }

            std::sort(aFound.begin(), aFound.end());
            for (const auto& rFound : aFound)
                aOut.push_back(rFound.second);
            break;
        }

        case XML_des:
        {
            std::set<OUString> aSeen;
            gatherDescendants(rDgm, rFrom, aSeen, aOut);
            break;
        }

        default:
            break;
    }

    return aOut;
}

/**
 * The presentation Point rId hangs off, an empty string at the top.
 */
OUString presentationParentOf(const SmartArtDiagram& rDgm, std::u16string_view rId)
{
    for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             rDgm.getData()->getConnections())
        if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_presParOf
            && rConnection->msDestId == rId)
            return rConnection->msSourceId;

    return OUString();
}

/**
 * The presentation Points that hang off rId, in the order the file puts them in.
 */
std::vector<OUString> presentationChildrenOf(const SmartArtDiagram& rDgm, std::u16string_view rId)
{
    std::vector<std::pair<sal_Int32, OUString>> aFound;
    for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             rDgm.getData()->getConnections())
        if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_presParOf
            && rConnection->msSourceId == rId)
            aFound.emplace_back(rConnection->mnSourceOrder, rConnection->msDestId);

    std::sort(aFound.begin(), aFound.end());

    std::vector<OUString> aOut;
    for (const auto& rFound : aFound)
        aOut.push_back(rFound.second);

    return aOut;
}

/**
 * The first presentation Point below rId, rId itself included, whose layout node is named rName.
 */
rtl::Reference<svx::diagram::Point> presentationNamedBelow(const SmartArtDiagram& rDgm,
                                                            std::u16string_view rId,
                                                            std::u16string_view rName)
{
    const rtl::Reference<svx::diagram::Point> xPoint(rDgm.getData()->getPointByModelID(rId));
    if (xPoint.is() && xPoint->getPresentation().msPresentationLayoutName == rName)
        return xPoint;

    for (const OUString& rChild : presentationChildrenOf(rDgm, rId))
    {
        const rtl::Reference<svx::diagram::Point> xBelow(
            presentationNamedBelow(rDgm, rChild, rName));
        if (xBelow.is())
            return xBelow;
    }

    return rtl::Reference<svx::diagram::Point>();
}

/**
 * The nearest presentation Point named rName to one side of rFromId: the siblings on that side
 * are searched first, nearest first and each with all that hangs below it, then the same is done
 * one level up, until the top is reached.
 */
rtl::Reference<svx::diagram::Point> presentationNamedBeside(const SmartArtDiagram& rDgm,
                                                             const OUString& rFromId,
                                                             std::u16string_view rName,
                                                             bool bBefore)
{
    OUString aStanding(rFromId);
    for (OUString aParent(presentationParentOf(rDgm, aStanding)); !aParent.isEmpty();
         aStanding = aParent, aParent = presentationParentOf(rDgm, aStanding))
    {
        const std::vector<OUString> aSiblings(presentationChildrenOf(rDgm, aParent));
        const auto aOwn = std::find(aSiblings.begin(), aSiblings.end(), aStanding);
        if (aOwn == aSiblings.end())
            continue;

        if (bBefore)
        {
            for (auto aIt = aOwn; aIt != aSiblings.begin();)
            {
                --aIt;
                const rtl::Reference<svx::diagram::Point> xFound(
                    presentationNamedBelow(rDgm, *aIt, rName));
                if (xFound.is())
                    return xFound;
            }
        }
        else
        {
            for (auto aIt = aOwn + 1; aIt != aSiblings.end(); ++aIt)
            {
                const rtl::Reference<svx::diagram::Point> xFound(
                    presentationNamedBelow(rDgm, *aIt, rName));
                if (xFound.is())
                    return xFound;
            }
        }
    }

    return rtl::Reference<svx::diagram::Point>();
}

/**
 * Where the shape of presentation Point rId stands on the page: its own place, and the place
 * of every group it hangs in, added up. Empty where there is no shape for it.
 */
std::optional<awt::Point> absolutePlaceOf(const SmartArtDiagram& rDgm, const OUString& rId)
{
    const PresPointShapeMap& rShapes(rDgm.getLayout()->getPresPointShapeMap());
    awt::Point aPlace(0, 0);
    bool bAny(false);

    for (OUString aId(rId); !aId.isEmpty(); aId = presentationParentOf(rDgm, aId))
    {
        const rtl::Reference<svx::diagram::Point> xPoint(rDgm.getData()->getPointByModelID(aId));
        const auto aShape = xPoint.is() ? rShapes.find(xPoint) : rShapes.end();
        if (aShape == rShapes.end() || !aShape->second)
            continue;

        aPlace.X += aShape->second->getPosition().X;
        aPlace.Y += aShape->second->getPosition().Y;
        bAny = true;
    }

    if (!bAny)
        return std::nullopt;

    return aPlace;
}

sal_Int32 calcMaxDepth(std::u16string_view rNodeName, const svx::diagram::Connections& rConnections)
{
    sal_Int32 nMaxLength = 0;
    for (const rtl::Reference<svx::diagram::Connection>& aCxn : rConnections)
        if (aCxn->mnXMLType == svx::diagram::TypeConstant::XML_parOf
            && aCxn->msSourceId == rNodeName)
            nMaxLength = std::max(nMaxLength, calcMaxDepth(aCxn->msDestId, rConnections) + 1);

    return nMaxLength;
}
}

std::vector<OUString> pointsAlongAxis(const SmartArtDiagram& rDgm, const IteratorAttr& rIterator,
                                      const OUString& rFromId, bool bLastStepWhole)
{
    // An axis is walked one step at a time, and what a step reaches is where the next one starts
    // from. What the last step reaches is the answer, so a single ch gives the children and a
    // followSib gives what comes after, which is not the children of anything.
    std::vector<OUString> aStanding{ rFromId };

    for (size_t nStep = 0; nStep < rIterator.maAxis.size(); ++nStep)
    {
        // A step names the kind of Point it wants, and all stands for every kind.
        const sal_Int32 nWanted(nStep < rIterator.maPtType.size() ? rIterator.maPtType[nStep]
                                                                  : XML_all);

        std::vector<OUString> aReached;
        for (const OUString& rFrom : aStanding)
            for (const OUString& rHit :
                     stepOfAxis(rDgm, rFrom, rIterator.maAxis[nStep], nWanted))
                aReached.push_back(rHit);

        std::erase_if(aReached, [&rDgm, nWanted](const OUString& rId) {
            return !isPointOfKind(typeOfPoint(rDgm, rId), nWanted);
        });

        // A step can say where among what it reached to start, counted from one, and how many to
        // take from there. A step that takes all of them leaves both out, and writes the count as
        // zero where it writes it at all. The last step of a loop is left whole, the loop has a
        // start and a count of its own and applies them to the passes it makes.
        const bool bWhole(bLastStepWhole && nStep + 1 == rIterator.maAxis.size());
        const bool bHasStart(!bWhole && nStep < rIterator.maStart.size());
        const bool bHasTake(!bWhole && nStep < rIterator.maCount.size());
        const sal_Int32 nStart(bHasStart ? rIterator.maStart[nStep] : 0);
        const sal_Int32 nTake(bHasTake ? rIterator.maCount[nStep] : 0);

        if (nStart > 1)
            aReached.erase(aReached.begin(),
                           aReached.begin() + std::min<size_t>(nStart - 1, aReached.size()));

        if (nTake > 0 && o3tl::make_unsigned(nTake) < aReached.size())
            aReached.resize(nTake);

        aStanding = std::move(aReached);
    }

    return aStanding;
}

namespace
{
/**
 * Every layout node of the layout by its name.
 */

// The algorithms stated for the layout node rAtom itself, in every branch of a choose, and not
// the ones of the layout nodes below it.
void gatherOwnAlgorithms(const LayoutAtom& rAtom, std::vector<const AlgAtom*>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
            rOut.push_back(pAlg);
        else
            gatherOwnAlgorithms(*pChild, rOut);
    }
}

// True when every layout node directly below rRowNode is a spacer, one laid out by the sp
// algorithm whichever branch of a choose is taken, a visible shape or none.
bool hasOnlySpacerChildren(const LayoutNode& rRowNode)
{
    bool bAny(false);
    for (const LayoutAtomPtr& pChild : rRowNode.getChildren())
    {
        const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get());
        if (!pNode)
            continue;
        std::vector<const AlgAtom*> aAlgorithms;
        gatherOwnAlgorithms(*pNode, aAlgorithms);
        if (aAlgorithms.empty())
            return false;
        for (const AlgAtom* pAlg : aAlgorithms)
            if (pAlg->getType() != XML_sp)
                return false;
        bAny = true;
    }
    return bAny;
}

// True when a hierChild or hierRoot algorithm sits anywhere below rAtom, in whichever branch
// of a choose.
bool hasHierarchyBelow(const LayoutAtom& rAtom)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
            if (pAlg->getType() == XML_hierChild || pAlg->getType() == XML_hierRoot)
                return true;
        if (hasHierarchyBelow(*pChild))
            return true;
    }
    return false;
}

// The names of the layout nodes a connector runs from or to, the srcNode and the dstNode of every
// connector algorithm from rAtom down.
void gatherConnectorEnds(const LayoutAtom& rAtom, std::set<OUString>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
        {
            for (const sal_Int32 nEnd : { XML_srcNode, XML_dstNode })
            {
                const OUString aName(pAlg->getNamedParam(nEnd));
                if (!aName.isEmpty())
                    rOut.insert(aName);
            }
        }
        gatherConnectorEnds(*pChild, rOut);
    }
}
}

namespace
{
/**
 * Where a connection site lies on a shape standing at rAt with size rSize. The sites are the
 * middle of each edge, the corners and the centre; anything else, auto among them, is the centre.
 */
awt::Point siteOn(sal_Int32 nSite, const awt::Point& rAt, const awt::Size& rSize)
{
    const sal_Int32 nLeft(rAt.X), nRight(rAt.X + rSize.Width), nMidX(rAt.X + rSize.Width / 2);
    const sal_Int32 nTop(rAt.Y), nBottom(rAt.Y + rSize.Height), nMidY(rAt.Y + rSize.Height / 2);
    switch (nSite)
    {
        case XML_midL: return awt::Point(nLeft, nMidY);
        case XML_midR: return awt::Point(nRight, nMidY);
        case XML_tCtr: return awt::Point(nMidX, nTop);
        case XML_bCtr: return awt::Point(nMidX, nBottom);
        case XML_tL: return awt::Point(nLeft, nTop);
        case XML_tR: return awt::Point(nRight, nTop);
        case XML_bL: return awt::Point(nLeft, nBottom);
        case XML_bR: return awt::Point(nRight, nBottom);
        default: return awt::Point(nMidX, nMidY);
    }
}

const AlgAtom* algorithmOf(const SmartArtDiagram& rDgm, const LayoutAtom& rAtom,
                           const rtl::Reference<svx::diagram::Point>& rPoint)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;

        if (const AlgAtom* pFound = dynamic_cast<const AlgAtom*>(pChild.get()))
            return pFound;

        if (dynamic_cast<const ChooseAtom*>(pChild.get()))
        {
            for (const LayoutAtomPtr& pBranch : pChild->getChildren())
            {
                const ConditionAtom* pCondition
                    = dynamic_cast<const ConditionAtom*>(pBranch.get());
                if (pCondition && pCondition->getDecision(rDgm, rPoint, OUString()))
                    return algorithmOf(rDgm, *pBranch, rPoint);
            }
            continue;
        }

        if (const AlgAtom* pBelow = algorithmOf(rDgm, *pChild, rPoint))
            return pBelow;
    }

    return nullptr;
}

// The constraints a layout node states for itself, read the way the layout read them for
// rPoint: a choose in between is decided, and only the branch taken is walked. The walk stops at a
// layout node of its own.
void gatherDecidedConstraints(const SmartArtDiagram& rDgm, const LayoutAtom& rAtom,
                              const rtl::Reference<svx::diagram::Point>& rPoint,
                              std::vector<Constraint>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const ConstraintAtom* pConstraint = dynamic_cast<const ConstraintAtom*>(pChild.get()))
        {
            pConstraint->parseConstraint(rOut, /*bRequireForName*/ false);
            continue;
        }
        if (dynamic_cast<const ChooseAtom*>(pChild.get()))
        {
            for (const LayoutAtomPtr& pBranch : pChild->getChildren())
            {
                const ConditionAtom* pCondition
                    = dynamic_cast<const ConditionAtom*>(pBranch.get());
                if (pCondition && pCondition->getDecision(rDgm, rPoint, OUString()))
                {
                    gatherDecidedConstraints(rDgm, *pBranch, rPoint, rOut);
                    break;
                }
            }
            continue;
        }
        gatherDecidedConstraints(rDgm, *pChild, rPoint, rOut);
    }
}

bool isEdgeSite(sal_Int32 nSite)
{
    return nSite == XML_midL || nSite == XML_midR || nSite == XML_tCtr || nSite == XML_bCtr
           || nSite == XML_tL || nSite == XML_tR || nSite == XML_bL || nSite == XML_bR;
}
}

// True for a connector that spans the ring it stands on, its diam stated as the diam of the
// ring by the node that draws the ring, by the connector's name or for every sibTrans. Such a
// connector is the ring itself, an arc round all of its nodes.
static bool spansItsRing(const SmartArtDiagram& rDgm,
                         const std::map<OUString, const LayoutNode*>& rNodes,
                         const rtl::Reference<svx::diagram::Point>& xOwn)
{
    const rtl::Reference<svx::diagram::Point> xRing(
        rDgm.getData()->getPointByModelID(presentationParentOf(rDgm, xOwn->msModelId)));
    if (!xRing.is())
        return false;

    const auto aRing = rNodes.find(xRing->getPresentation().msPresentationLayoutName);
    if (aRing == rNodes.end() || !aRing->second)
        return false;

    const OUString& rOwnName(xOwn->getPresentation().msPresentationLayoutName);
    for (const auto& pChild : aRing->second->getChildren())
    {
        const auto pConstraintAtom = dynamic_cast<ConstraintAtom*>(pChild.get());
        if (!pConstraintAtom)
            continue;
        const Constraint& rConstraint(pConstraintAtom->getConstraint());
        if (XML_diam == rConstraint.mnType && XML_diam == rConstraint.mnRefType
            && (rConstraint.msForName == rOwnName
                || (rConstraint.msForName.isEmpty() && XML_sibTrans == rConstraint.mnPointType)))
            return true;
    }
    return false;
}

// The drawing of the other office stands in the middle of the frame: of 73 files of the corpus
// whose drawn shapes are narrower than the frame 65 have them centred, of 84 lower than it 74,
// and the rest are the ones whose root algorithm aligns the whole to a side, vertAlign t of
// Square_Accent_List and Increasing_Circle_Process; nodeVertAlign t of Detailed_Process and
// Stacked_List aligns the nodes in their row and the whole is centred all the same. The layout
// works with the boxes, and a box is often wider than what is drawn in it, Vertical_Accent_
// List's composite of 6104k in a column of 8128k; so the whole is moved once the shapes have
// their place, by the drawn shapes, the ones with a geometry or a connector, and not by the
// boxes. A turned shape counts with the box it covers turned, Numbered_List's boxes of 7501600
// standing on their side; a shape that reaches out of the frame, Vertical_Curved_List's arc
// that is mostly left of it, counts up to the frame's edge.
void centreDrawnShapes(const SmartArtDiagram& rDgm, const ShapePtr& pRoot)
{
    if (!pRoot || !rDgm.getLayout() || !rDgm.getLayout()->getNode())
        return;
    if (pRoot->getSize().Width <= 0 || pRoot->getSize().Height <= 0)
        return;

    bool bAlongX(true);
    bool bAlongY(true);
    std::vector<const AlgAtom*> aAlgorithms;
    gatherOwnAlgorithms(*rDgm.getLayout()->getNode(), aAlgorithms);
    for (const AlgAtom* pAlg : aAlgorithms)
    {
        const auto aSaid = [pAlg](sal_Int32 nParam, sal_Int32 nMiddle) {
            const auto aFound = pAlg->getMap().find(nParam);
            return aFound != pAlg->getMap().end() && aFound->second != nMiddle;
        };
        if (aSaid(XML_horzAlign, XML_ctr))
            bAlongX = false;
        if (aSaid(XML_vertAlign, XML_mid))
            bAlongY = false;
    }
    if (!bAlongX && !bAlongY)
        return;

    sal_Int32 nLeft(std::numeric_limits<sal_Int32>::max());
    sal_Int32 nTop(std::numeric_limits<sal_Int32>::max());
    sal_Int32 nRight(std::numeric_limits<sal_Int32>::min());
    sal_Int32 nBottom(std::numeric_limits<sal_Int32>::min());
    bool bAny(false);
    const auto aGather = [&](const ShapePtr& pShape, awt::Point aAt, auto& rSelf) -> void {
        for (const ShapePtr& pChild : pShape->getChildren())
        {
            const awt::Point aChildAt(aAt.X + pChild->getPosition().X,
                                      aAt.Y + pChild->getPosition().Y);
            // a group is a box, a shape without a preset geometry is hidden, a connector and
            // every shape with a geometry are drawn; a transition drawn by a connector is not
            // centred
            const bool bDrawn(pChild->getServiceName() != "com.sun.star.drawing.GroupShape"
                              && (pChild->getSubType() == XML_conn
                                  || pChild->getCustomShapeProperties()->getShapePresetType()
                                         != 0)
                              && !((pChild->getDataNodeType() == XML_sibTrans
                                    || pChild->getDataNodeType() == XML_parTrans)
                                   && (pChild->getSubType() == XML_conn
                                       || pChild->getSubType() == XML_circularArrow
                                       || pChild->getSubType() == XML_leftCircularArrow))
                              && pChild->getSize().Width > 0 && pChild->getSize().Height > 0);
            if (bDrawn)
            {
                // A turn of the layout by a right angle is still on the shape here, the sizing
                // swaps its sides later, and the box is the box it covers as it is: Vertical_
                // Chevron_List's chevron of 1024214 by 1463163 turned by 90 degrees covers just
                // that, not 1463163 by 1024214, which made the union 5051k wide and put the
                // whole 1538k to the right of the drawing.
                const sal_Int32 nDiagramTurn(pChild->getDiagramRotation());
                const double fAngle(basegfx::deg2rad<60000>(
                    pChild->getRotation()
                    + (nDiagramTurn % (90 * 60000) == 0 ? 0 : nDiagramTurn)));
                const double fCos(std::abs(cos(fAngle)));
                const double fSin(std::abs(sin(fAngle)));
                double fHalfWide(
                    (pChild->getSize().Width * fCos + pChild->getSize().Height * fSin) / 2.0);
                double fHalfHigh(
                    (pChild->getSize().Width * fSin + pChild->getSize().Height * fCos) / 2.0);
                // a band round the ring (a blockArc transition) counts as the ring's circle plus
                // the connector's thickness, the band being 0.6 of it, as the ring was fitted
                if (pChild->getSubType() == XML_blockArc
                    && (pChild->getDataNodeType() == XML_sibTrans
                        || pChild->getDataNodeType() == XML_parTrans))
                {
                    const auto& rGuides(
                        pChild->getCustomShapeProperties()->getAdjustmentGuideList());
                    const sal_Int32 nAt(rGuides.GetCustomShapeGuideValue(u"adj3"_ustr));
                    const sal_Int32 nBandPart(nAt >= 0 ? rGuides[nAt].maFormula.toInt32() : 0);
                    if (nBandPart > 0)
                    {
                        const double fBox(std::min(pChild->getSize().Width,
                                                   pChild->getSize().Height));
                        const double fBand(fBox * nBandPart / 100000.0);
                        const double fReach(fBox / 2.0 - fBand / 4.0 + fBand / 0.6);
                        fHalfWide = std::max(fHalfWide, fReach);
                        fHalfHigh = std::max(fHalfHigh, fReach);
                    }
                }
                const double fMiddleX(aChildAt.X + pChild->getSize().Width / 2.0);
                const double fMiddleY(aChildAt.Y + pChild->getSize().Height / 2.0);
                nLeft = std::min<sal_Int32>(nLeft, std::max<sal_Int32>(0, fMiddleX - fHalfWide));
                nTop = std::min<sal_Int32>(nTop, std::max<sal_Int32>(0, fMiddleY - fHalfHigh));
                nRight = std::max<sal_Int32>(
                    nRight, std::min<sal_Int32>(pRoot->getSize().Width, fMiddleX + fHalfWide));
                nBottom = std::max<sal_Int32>(
                    nBottom, std::min<sal_Int32>(pRoot->getSize().Height, fMiddleY + fHalfHigh));
                bAny = true;
            }
            rSelf(pChild, aChildAt, rSelf);
        }
    };
    aGather(pRoot, awt::Point(0, 0), aGather);
    if (!bAny)
        return;

    // The shift is kept on the drawing's grid of hundredths of a millimetre, 360 EMU, so that
    // the edges of the shapes round to the same hundredths wherever the middle comes to lie.
    const auto aOnGrid = [](sal_Int32 nShift) {
        return static_cast<sal_Int32>(std::round(nShift / 360.0) * 360.0);
    };
    const sal_Int32 nShiftX(
        bAlongX ? aOnGrid((pRoot->getSize().Width - (nRight - nLeft)) / 2 - nLeft) : 0);
    const sal_Int32 nShiftY(
        bAlongY ? aOnGrid((pRoot->getSize().Height - (nBottom - nTop)) / 2 - nTop) : 0);
    if (nShiftX == 0 && nShiftY == 0)
        return;
    for (const ShapePtr& pChild : pRoot->getChildren())
        pChild->setPosition(awt::Point(pChild->getPosition().X + nShiftX,
                                       pChild->getPosition().Y + nShiftY));
}

// A node that states its nodes as alike along an axis, "h for des ptType=node op=equ" with no
// reference, has every node come out the size of the smallest once the rows have fitted them:
// Table_Hierarchy's top node without children stands alone in its column and would be the whole
// height, 15052, where the drawing has it 7217 like the nodes beside it, each of those above a
// gap and a row of its children. The nodes are the drawn shapes of a node's data below the
// stating node's shape, its direct children for ch, and a node keeps the edge the row holding it
// runs from, its bottom in a column fromB.
void settleEqualNodeSizes(const SmartArtDiagram& rDgm, const ShapePtr& pRoot)
{
    if (!pRoot || !rDgm.getLayout() || !rDgm.getLayout()->getNode())
        return;
    const LayoutNode& rRootNode(*rDgm.getLayout()->getNode());
    std::map<OUString, const LayoutNode*> aNodes;
    gatherLayoutNodes(rRootNode, aNodes);
    aNodes[rRootNode.getName()] = &rRootNode;

    // the row each shape stands in, by the shape
    std::map<const Shape*, const Shape*> aRowOf;
    const auto aGatherRows = [&aRowOf](const ShapePtr& pShape, auto& rSelf) -> void {
        for (const ShapePtr& pChild : pShape->getChildren())
        {
            aRowOf[pChild.get()] = pShape.get();
            rSelf(pChild, rSelf);
        }
    };
    aGatherRows(pRoot, aGatherRows);

    const auto aIsDrawnNode = [](const ShapePtr& pShape) {
        return pShape->getServiceName() != "com.sun.star.drawing.GroupShape"
               && pShape->getCustomShapeProperties()->getShapePresetType() != 0
               && pShape->getDataNodeType() == XML_node && pShape->getSize().Width > 0
               && pShape->getSize().Height > 0;
    };

    for (const auto& rEntry : aNodes)
    {
        if (!rEntry.second)
            continue;
        for (const Constraint& rConstraint : collectDirectConstraints(*rEntry.second))
        {
            if ((rConstraint.mnType != XML_w && rConstraint.mnType != XML_h)
                || (rConstraint.mnFor != XML_des && rConstraint.mnFor != XML_ch)
                || rConstraint.mnRefType != XML_none || !rConstraint.msRefForName.isEmpty()
                || rConstraint.mnOperator != XML_equ)
                continue;
            // The nodes stated alike by a name, "h for des forName desTx op equ", are the
            // shapes of that name, and where a row grew them to their texts they all take the
            // largest: Continuous_Arrow_Process's child texts of two lines are as high as the
            // one of three, 1575000. Stated by the point type they take the smallest, see
            // above.
            const bool bByName(!rConstraint.msForName.isEmpty());
            if (bByName
                && !const_cast<SmartArtDiagram&>(rDgm).getFittedFontSizes().count(
                    rConstraint.msForName))
                continue;
            if (!bByName && rConstraint.mnPointType != XML_node)
                continue;
            const bool bWidth(rConstraint.mnType == XML_w);

            // the shapes of the stating node, the root's being the root itself
            std::vector<ShapePtr> aStating;
            if (rEntry.second == &rRootNode)
                aStating.push_back(pRoot);
            const auto aFindStating = [&](const ShapePtr& pShape, auto& rSelf) -> void {
                for (const ShapePtr& pChild : pShape->getChildren())
                {
                    if (pChild->getInternalName() == rEntry.first)
                        aStating.push_back(pChild);
                    rSelf(pChild, rSelf);
                }
            };
            if (rEntry.second != &rRootNode)
                aFindStating(pRoot, aFindStating);

            for (const ShapePtr& pStating : aStating)
            {
                std::vector<ShapePtr> aAlike;
                const auto aGather = [&](const ShapePtr& pShape, auto& rSelf) -> void {
                    for (const ShapePtr& pChild : pShape->getChildren())
                    {
                        if (bByName ? pChild->getInternalName() == rConstraint.msForName
                                    : aIsDrawnNode(pChild))
                            aAlike.push_back(pChild);
                        if (rConstraint.mnFor == XML_des)
                            rSelf(pChild, rSelf);
                    }
                };
                aGather(pStating, aGather);
                if (aAlike.size() < 2)
                    continue;
                sal_Int32 nAlike(bByName ? 0 : std::numeric_limits<sal_Int32>::max());
                for (const ShapePtr& pOne : aAlike)
                {
                    const sal_Int32 nSide(bWidth ? pOne->getSize().Width : pOne->getSize().Height);
                    nAlike = bByName ? std::max(nAlike, nSide) : std::min(nAlike, nSide);
                }
                for (const ShapePtr& pOne : aAlike)
                {
                    awt::Size aSize(pOne->getSize());
                    const sal_Int32 nBefore(bWidth ? aSize.Width : aSize.Height);
                    if (nBefore == nAlike || (!bByName && nBefore < nAlike))
                        continue;
                    // the direction of the row the node stands in, fromL where none is stated
                    sal_Int32 nDirection(XML_fromL);
                    const auto aRow = aRowOf.find(pOne.get());
                    const auto aRowNode
                        = aRow != aRowOf.end() ? aNodes.find(aRow->second->getInternalName())
                                               : aNodes.end();
                    if (aRowNode != aNodes.end() && aRowNode->second)
                    {
                        std::vector<const AlgAtom*> aAlgorithms;
                        gatherOwnAlgorithms(*aRowNode->second, aAlgorithms);
                        for (const AlgAtom* pAlg : aAlgorithms)
                        {
                            const auto aDir = pAlg->getMap().find(XML_linDir);
                            if (pAlg->getType() == XML_lin && aDir != pAlg->getMap().end())
                                nDirection = aDir->second;
                        }
                    }
                    awt::Point aAt(pOne->getPosition());
                    if (bWidth)
                    {
                        aSize.Width = nAlike;
                        if (nDirection == XML_fromR)
                            aAt.X += nBefore - nAlike;
                    }
                    else
                    {
                        aSize.Height = nAlike;
                        if (nDirection == XML_fromB)
                            aAt.Y += nBefore - nAlike;
                    }
                    pOne->setSize(aSize);
                    pOne->setChildSize(aSize);
                    pOne->setPosition(aAt);
                }
            }
        }
    }
}

// A child whose side is bound both ways to a part of a shape levels below a sibling, "h for ch
// backgroundArrow refType h refFor des parTx op lte fact 2" and the same with gte, is that part
// of the shape's side once the shape is laid out, and keeps its middle: Continuous_Arrow_
// Process's arrow is twice as high as a step's text, 7000 for 3500, whatever the text came to.
void settleSidesBoundToNamedShapes(const SmartArtDiagram& rDgm, const ShapePtr& pRoot)
{
    if (!pRoot || !rDgm.getLayout() || !rDgm.getLayout()->getNode())
        return;
    std::map<OUString, const LayoutNode*> aNodes;
    gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);
    aNodes[rDgm.getLayout()->getNode()->getName()] = rDgm.getLayout()->getNode().get();
    for (const auto& rEntry : aNodes)
    {
        if (!rEntry.second)
            continue;
        // the node's own bounds, the branches of a choose included; the plain gathering keeps
        // the equalities alone
        std::vector<Constraint> aOwn;
        std::function<void(const LayoutAtom&)> aBounds = [&](const LayoutAtom& rAtom) {
            for (const LayoutAtomPtr& pBelow : rAtom.getChildren())
            {
                if (dynamic_cast<const LayoutNode*>(pBelow.get()))
                    continue;
                if (const ConstraintAtom* pConstraint
                    = dynamic_cast<const ConstraintAtom*>(pBelow.get()))
                {
                    if (pConstraint->getConstraint().mnOperator == XML_lte
                        || pConstraint->getConstraint().mnOperator == XML_gte)
                        aOwn.push_back(pConstraint->getConstraint());
                }
                else
                    aBounds(*pBelow);
            }
        };
        aBounds(*rEntry.second);
        for (const Constraint& rLower : aOwn)
        {
            if (rLower.mnOperator != XML_gte
                || (rLower.mnFor != XML_ch && rLower.mnFor != XML_des)
                || rLower.msForName.isEmpty() || rLower.mnRefFor != XML_des
                || rLower.msRefForName.isEmpty() || rLower.mfFactor <= 0.0
                || (rLower.mnType != XML_w && rLower.mnType != XML_h)
                || (rLower.mnRefType != XML_w && rLower.mnRefType != XML_h))
                continue;
            bool bUpperToo(false);
            for (const Constraint& rUpper : aOwn)
                if (rUpper.mnOperator == XML_lte && rUpper.mnType == rLower.mnType
                    && rUpper.mnRefType == rLower.mnRefType
                    && rUpper.msForName == rLower.msForName
                    && rUpper.msRefForName == rLower.msRefForName
                    && std::abs(rUpper.mfFactor - rLower.mfFactor) < 1e-9)
                    bUpperToo = true;
            if (!bUpperToo)
                continue;
            const std::optional<sal_Int32> oOf(
                sizeOfLaidOutShape(rDgm, rLower.msRefForName, rLower.mnRefType));
            if (!oOf || *oOf <= 0)
                continue;
            const sal_Int32 nSide(static_cast<sal_Int32>(*oOf * rLower.mfFactor));
            // A child of the stating row stands in the middle of the shape it is bound to along
            // that axis, Continuous_Arrow_Process's arrow on the step texts; a shape levels
            // below, "for des", was placed by its composite and keeps its top, Accent_Process's
            // parSh at the top of its node like the text it is 1.5 of.
            std::optional<sal_Int32> oMiddleOfBound;
            for (const auto& rPoint : rDgm.getLayout()->getPresPointShapeMap())
            {
                if (rLower.mnFor != XML_ch)
                    break;
                if (!rPoint.second || rPoint.second->getInternalName() != rLower.msRefForName
                    || !rPoint.first.is() || rPoint.second->getSize().Width <= 0)
                    continue;
                const std::optional<awt::Point> oAt(absolutePlaceOf(rDgm, rPoint.first->msModelId));
                if (!oAt)
                    continue;
                oMiddleOfBound = rLower.mnType == XML_w
                                     ? oAt->X + rPoint.second->getSize().Width / 2
                                     : oAt->Y + rPoint.second->getSize().Height / 2;
                break;
            }
            const auto aSettle = [&](const ShapePtr& pShape, const awt::Point& rShapeAt,
                                     auto& rSelf) -> void {
                for (const ShapePtr& pChild : pShape->getChildren())
                {
                    const awt::Point aChildAt(rShapeAt.X + pChild->getPosition().X,
                                              rShapeAt.Y + pChild->getPosition().Y);
                    if (pChild->getInternalName() == rLower.msForName)
                    {
                        awt::Size aSize(pChild->getSize());
                        awt::Point aAt(pChild->getPosition());
                        if (rLower.mnType == XML_w)
                        {
                            if (rLower.mnFor == XML_ch)
                                aAt.X += (aSize.Width - nSide) / 2;
                            if (oMiddleOfBound)
                                aAt.X = *oMiddleOfBound - nSide / 2 - rShapeAt.X;
                            aSize.Width = nSide;
                        }
                        else
                        {
                            if (rLower.mnFor == XML_ch)
                                aAt.Y += (aSize.Height - nSide) / 2;
                            if (oMiddleOfBound)
                                aAt.Y = *oMiddleOfBound - nSide / 2 - rShapeAt.Y;
                            aSize.Height = nSide;
                        }
                        pChild->setSize(aSize);
                        pChild->setChildSize(aSize);
                        pChild->setPosition(aAt);
                    }
                    rSelf(pChild, aChildAt, rSelf);
                }
            };
            aSettle(pRoot, awt::Point(0, 0), aSettle);
        }
    }
}

// A spacer in a row whose extent is a part of a shape that a hierarchy beside it lays out gets
// that extent once the hierarchy has done so, and what follows it in the row moves along by
// it: Labeled_Hierarchy's firstBuf of 0.1 of level1Shape stands above the first level, and the
// level stands 102261 below the band's top for a level of 1022614.
void settleDeferredSpacers(const SmartArtDiagram& rDgm)
{
    for (const SmartArtDiagram::DeferredSpacer& rSpacer :
         const_cast<SmartArtDiagram&>(rDgm).getDeferredSpacers())
    {
        if (!rSpacer.mpRow || !rSpacer.mpSpacer)
            continue;
        // the shape referred to, the first of its name among the row's descendants
        const auto aFind = [&rSpacer](const ShapePtr& pShape, auto& rSelf) -> ShapePtr {
            for (const ShapePtr& pChild : pShape->getChildren())
            {
                if (pChild->getInternalName() == rSpacer.maRefName && pChild->getSize().Width > 0
                    && pChild->getSize().Height > 0)
                    return pChild;
                if (const ShapePtr pBelow = rSelf(pChild, rSelf))
                    return pBelow;
            }
            return ShapePtr();
        };
        const ShapePtr pReferred(aFind(rSpacer.mpRow, aFind));
        if (!pReferred)
            continue;
        const sal_Int32 nOf(rSpacer.mnRefType == XML_w ? pReferred->getSize().Width
                                                       : pReferred->getSize().Height);
        const sal_Int32 nExtent(static_cast<sal_Int32>(nOf * rSpacer.mfFactor));
        if (nExtent <= 0)
            continue;
        awt::Size aSpacerSize(rSpacer.mpSpacer->getSize());
        const sal_Int32 nBefore(rSpacer.mbAlongX ? aSpacerSize.Width : aSpacerSize.Height);
        const sal_Int32 nMove(nExtent - nBefore);
        if (rSpacer.mbAlongX)
            aSpacerSize.Width = nExtent;
        else
            aSpacerSize.Height = nExtent;
        rSpacer.mpSpacer->setSize(aSpacerSize);
        rSpacer.mpSpacer->setChildSize(aSpacerSize);
        if (nMove == 0)
            continue;
        // the row may have taken the spacer out of its children since, a space among children
        // without wishes; then the children from its place on are the ones after it
        const bool bSpacerStays(
            std::find(rSpacer.mpRow->getChildren().begin(), rSpacer.mpRow->getChildren().end(),
                      rSpacer.mpSpacer)
            != rSpacer.mpRow->getChildren().end());
        bool bAfter(false);
        sal_Int32 nAt(0);
        for (const ShapePtr& pChild : rSpacer.mpRow->getChildren())
        {
            if (!bSpacerStays && nAt++ >= rSpacer.mnIndex)
                bAfter = true;
            if (pChild == rSpacer.mpSpacer)
            {
                bAfter = true;
                if (rSpacer.mnDirection < 0)
                {
                    // a row that runs back has the spacer grow toward its start
                    awt::Point aAt(pChild->getPosition());
                    if (rSpacer.mbAlongX)
                        aAt.X -= nMove;
                    else
                        aAt.Y -= nMove;
                    pChild->setPosition(aAt);
                }
                continue;
            }
            if (!bAfter)
                continue;
            awt::Point aAt(pChild->getPosition());
            if (rSpacer.mbAlongX)
                aAt.X += nMove * rSpacer.mnDirection;
            else
                aAt.Y += nMove * rSpacer.mnDirection;
            pChild->setPosition(aAt);
        }
    }
}

// The text algorithm sets the pre-rotation of a shape's text from the shape's turn as it stands
// when the algorithm runs; a connector is turned by the snake or the settle pass afterwards,
// and its text comes from the data point it presents, not from a text algorithm of its own.
// Basic_Bending_Process's arrows carry "April", "Mai", "Juni", and the text is meant to stand
// upright on the turned arrows, autoTxRot upr on the connectorText beside them.
void settleUprightText(const SmartArtDiagram& rDgm)
{
    if (!rDgm.getLayout() || !rDgm.getLayout()->getNode())
        return;
    std::map<OUString, const LayoutNode*> aNodes;
    aNodes[rDgm.getLayout()->getNode()->getName()] = rDgm.getLayout()->getNode().get();
    gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);

    // the autoTxRot of the text algorithm of rNode, or none where it has no text algorithm
    const auto aTextRotationOf = [](const LayoutNode& rNode) -> std::optional<sal_Int32> {
        std::vector<const AlgAtom*> aAlgorithms;
        gatherOwnAlgorithms(rNode, aAlgorithms);
        for (const AlgAtom* pAlg : aAlgorithms)
            if (pAlg->getType() == XML_tx)
            {
                const auto aFound = pAlg->getMap().find(XML_autoTxRot);
                return aFound == pAlg->getMap().end() ? XML_upr : aFound->second;
            }
        return std::nullopt;
    };

    const auto& rShapes(rDgm.getLayout()->getPresPointShapeMap());
    for (const rtl::Reference<svx::diagram::Point>& xPoint : rDgm.getData()->getPoints())
    {
        const auto rEntry = rShapes.find(xPoint);
        if (rEntry == rShapes.end())
            continue;
        const ShapePtr& pShape(rEntry->second);
        // a shape laid out again has no text body here, its text comes back from the shape it
        // replaces, so the turn is kept on the shape itself
        if (!rEntry->first.is() || !pShape || pShape->getDiagramTextPreRotation()
            || (pShape->getTextBody()
                && pShape->getTextBody()->getTextProperties().moTextPreRotation.has_value()))
            continue;
        if (pShape->getServiceName() == "com.sun.star.drawing.GroupShape")
            continue;
        // the turn the shape is drawn with, its own plus the layout node's
        const sal_Int32 nTurn(
            (((pShape->getRotation() + pShape->getDiagramRotation()) / PER_DEGREE) % 360 + 360)
            % 360);
        if (nTurn == 0)
            continue;
        const auto aNode = aNodes.find(rEntry->first->getPresentation().msPresentationLayoutName);
        if (aNode == aNodes.end() || !aNode->second)
            continue;
        std::optional<sal_Int32> oRotation(aTextRotationOf(*aNode->second));
        if (!oRotation)
        {
            // a connector's text is styled by the text node beside it under the same parent
            const LayoutNode* pParent(aNode->second->getParentLayoutNode());
            if (pParent)
                for (const LayoutAtomPtr& pSibling : pParent->getChildren())
                {
                    const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pSibling.get());
                    if (!pNode || pNode == aNode->second)
                        continue;
                    oRotation = aTextRotationOf(*pNode);
                    if (oRotation)
                        break;
                }
        }
        if (!oRotation)
            oRotation = XML_upr;

        if (*oRotation == XML_upr)
        {
            int n90x = 0;
            if (nTurn >= 315)
                /* keep 0 */;
            else if (nTurn > 225)
                n90x = -3;
            else if (nTurn >= 135)
                n90x = -2;
            else if (nTurn > 45)
                n90x = -1;
            if (n90x != 0)
                pShape->setDiagramTextPreRotation(n90x * 90 * PER_DEGREE);
        }
        else if (*oRotation == XML_grav && nTurn > 90 && nTurn < 270)
            pShape->setDiagramTextPreRotation(-180 * PER_DEGREE);
    }
}

void settleNamedConnectors(const SmartArtDiagram& rDgm)
{
    const PresPointShapeMap& rShapes(rDgm.getLayout()->getPresPointShapeMap());

    // The connectors of one name that the root says share one connDist, "connDist for des
    // ptType sibTrans op equ" on Circular_Bending_Process, all take the shortest way among them
    // once every one has its own: its three triangles are 630126 long, the way between the
    // first two nodes less the pads, also the one between the second and the third that stand
    // 1917462 apart.
    struct SettledConnector
    {
        ShapePtr pShape;
        bool bHorizontal;
        bool bTurned;
    };
    std::map<OUString, std::vector<SettledConnector>> aSharingConnDist;
    std::set<OUString> aSharingNames;
    bool bSharingTransitions(false);
    if (rDgm.getLayout()->getNode())
    {
        // the root's own constraints, in every branch of a choose, not the nodes' below
        std::function<void(const LayoutAtom&)> aReadSharing
            = [&aReadSharing, &aSharingNames, &bSharingTransitions](const LayoutAtom& rAtom) {
                  for (const LayoutAtomPtr& pChild : rAtom.getChildren())
                  {
                      if (dynamic_cast<const LayoutNode*>(pChild.get()))
                          continue;
                      const ConstraintAtom* pConstraint
                          = dynamic_cast<const ConstraintAtom*>(pChild.get());
                      if (!pConstraint)
                      {
                          aReadSharing(*pChild);
                          continue;
                      }
                      const Constraint& rConstraint(pConstraint->getConstraint());
                      if (rConstraint.mnType != XML_connDist
                          || rConstraint.mnOperator != XML_equ)
                          continue;
                      if (!rConstraint.msForName.isEmpty())
                          aSharingNames.insert(rConstraint.msForName);
                      else if (rConstraint.mnPointType == XML_sibTrans)
                          bSharingTransitions = true;
                  }
              };
        aReadSharing(*rDgm.getLayout()->getNode());
    }

    std::map<OUString, const LayoutNode*> aNodes;
    if (rDgm.getLayout()->getNode())
    {
        aNodes[rDgm.getLayout()->getNode()->getName()] = rDgm.getLayout()->getNode().get();
        gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);
    }

    // The connectors are settled in the order of the data's points, the same on every build;
    // the map of the presentation points is ordered by their addresses.
    for (const rtl::Reference<svx::diagram::Point>& xOwn : rDgm.getData()->getPoints())
    {
        const auto aEntry = rShapes.find(xOwn);
        if (aEntry == rShapes.end())
            continue;
        const ShapePtr& pShape(aEntry->second);
        if (!xOwn.is() || !pShape)
            continue;

        const auto aNode = aNodes.find(xOwn->getPresentation().msPresentationLayoutName);
        if (aNode == aNodes.end() || !aNode->second)
            continue;

        // The algorithm of the layout node, which a choose may stand in front of. The choose is
        // read the way the layout read it, so the algorithm found is the one that laid the shape
        // out and not the one for the other direction.
        const AlgAtom* pAlg = algorithmOf(rDgm, *aNode->second, xOwn);
        if (!pAlg)
            continue;

        // A connector can name the shape it starts at and the one it ends at. The row it stands
        // in has settled where it goes along the flow, and across the flow it goes between those
        // two, which need not be where the middle of the row is. A bend is left as its branch
        // laid it, that one is an elbow drawn from the branch itself.
        const AlgAtom::ParamMap& rMap(pAlg->getMap());
        const sal_Int32 nRoute(rMap.count(XML_connRout) ? rMap.find(XML_connRout)->second
                                                        : XML_stra);
        const OUString aSourceName(pAlg->getNamedParam(XML_srcNode));
        const OUString aTargetName(pAlg->getNamedParam(XML_dstNode));

        // A curved line, connRout curve or longCurve, dim 1D, from one site to another is an
        // arc of a circle through both sites, whose diameter is diam as a part of connDist, the
        // way from the one site to the other, and that way itself where diam states nothing. The
        // centre lies on the right of the way from the start to the end, on the left for a diam
        // of less than nothing, and the shape is the square around the circle. Read off the
        // examples curve_vs_longCurve and Vertical_Curved_9: curve from bR to midL with no
        // diam, 3216793 for a way of 3216800, the centre in the middle; curveTop from tR to tL
        // with diam 1.5, 3236835, both sites 1618424 from the centre for a radius of 1618417.
        // The ends are the shapes the line names, or its neighbours in its row.
        // A line, dim 1D, that names the shape it ends at and not the one it starts at starts
        // at the shape drawn for the node its transition comes from: Sub-Step_Process's lines
        // run from the step's circle, midR, to the anchor of each sub-step's row, ending at the
        // middle of the edge that faces the circle, endPts being auto. begPad is a part of the
        // way left free at the start, 0.11 there, and the line is 443221 of a way of 498000.
        // The line lies flat and is turned to run along the way; it is as thick as the drawing
        // draws a line, nothing.
        // A line that names its start as well, the next step's circle for Sub-Step_Process's
        // lines back from a row's right edge, starts at the nearest shape of that name, before
        // it or after it.
        // Only a line a row, a lin, holds: a ring lays out its own spokes, Radial_List's.
        bool bInARow(false);
        {
            const rtl::Reference<svx::diagram::Point> xRowPoint(rDgm.getData()->getPointByModelID(
                presentationParentOf(rDgm, xOwn->msModelId)));
            const auto aRowNode = xRowPoint.is() ? aNodes.find(
                                      xRowPoint->getPresentation().msPresentationLayoutName)
                                                 : aNodes.end();
            const AlgAtom* pRowAlg(aRowNode != aNodes.end() && aRowNode->second
                                       ? algorithmOf(rDgm, *aRowNode->second, xRowPoint)
                                       : nullptr);
            bInARow = pRowAlg && pRowAlg->getType() == XML_lin;
        }
        if (bInARow && nRoute == XML_stra && !aTargetName.isEmpty() && rMap.count(XML_dim)
            && rMap.find(XML_dim)->second == XML_1D)
        {
            ShapePtr pFrom;
            rtl::Reference<svx::diagram::Point> xFrom;
            if (!aSourceName.isEmpty())
            {
                xFrom = presentationNamedBeside(rDgm, xOwn->msModelId, aSourceName,
                                                /*bBefore*/ true);
                if (!xFrom.is())
                    xFrom = presentationNamedBeside(rDgm, xOwn->msModelId, aSourceName,
                                                    /*bBefore*/ false);
                const auto aFrom = xFrom.is() ? rShapes.find(xFrom) : rShapes.end();
                if (aFrom != rShapes.end())
                    pFrom = aFrom->second;
                else
                    xFrom.clear();
            }
            const OUString aTransition(xOwn->getPresentation().msPresentationAssociationId);
            OUString aFromNode;
            for (const auto& rConnection : rDgm.getData()->getConnections())
                if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
                    && (rConnection->msParTransId == aTransition
                        || rConnection->msSibTransId == aTransition))
                {
                    aFromNode = rConnection->msSourceId;
                    break;
                }
            // the first shape drawn for that node in the order of the data's points: the map of
            // the presentation points is ordered by their addresses, and over it the first one
            // differed from one run to the next, so one of Sub-Step_Process's lines ran from
            // the circle of the step after its own
            for (const rtl::Reference<svx::diagram::Point>& xOther : rDgm.getData()->getPoints())
            {
                if (pFrom || aFromNode.isEmpty() || !xOther.is()
                    || xOther->getPresentation().msPresentationAssociationId != aFromNode)
                    continue;
                const auto aOther = rShapes.find(xOther);
                if (aOther == rShapes.end() || !aOther->second
                    || aOther->second->getCustomShapeProperties()->getShapePresetType() == 0
                    || aOther->second->getSubType() == XML_conn)
                    continue;
                pFrom = aOther->second;
                xFrom = xOther;
            }
            const rtl::Reference<svx::diagram::Point> xTo(presentationNamedBeside(
                rDgm, xOwn->msModelId, aTargetName, /*bBefore*/ false));
            const auto aTo = xTo.is() ? rShapes.find(xTo) : rShapes.end();
            const std::optional<awt::Point> aFromAt(
                xFrom.is() ? absolutePlaceOf(rDgm, xFrom->msModelId) : std::nullopt);
            const std::optional<awt::Point> aToAt(
                xTo.is() ? absolutePlaceOf(rDgm, xTo->msModelId) : std::nullopt);
            const std::optional<awt::Point> aParentAt(
                absolutePlaceOf(rDgm, presentationParentOf(rDgm, xOwn->msModelId)));
            // The target may be a spacer the row laid nothing out for, Sub-Step_Process's anchor
            // of the whole row that its backup walks back over: then its box is what the row's
            // constraints say, the stated part of the row's width from the row's start, and
            // the row's height.
            std::optional<awt::Point> aTargetAt(aToAt);
            awt::Size aTargetSize(aTo != rShapes.end() ? aTo->second->getSize() : awt::Size());
            if (aTo != rShapes.end() && aTargetSize.Width == 0 && aTargetSize.Height == 0)
            {
                const OUString aRowId(presentationParentOf(rDgm, xTo->msModelId));
                const rtl::Reference<svx::diagram::Point> xRow(
                    rDgm.getData()->getPointByModelID(aRowId));
                const auto aRow = xRow.is() ? rShapes.find(xRow) : rShapes.end();
                const std::optional<awt::Point> aRowAt(xRow.is() ? absolutePlaceOf(rDgm, aRowId)
                                                                 : std::nullopt);
                const auto aRowNode = xRow.is() ? aNodes.find(
                                          xRow->getPresentation().msPresentationLayoutName)
                                                : aNodes.end();
                if (aRow != rShapes.end() && aRowAt && aRowNode != aNodes.end()
                    && aRowNode->second)
                {
                    double fPart(1.0);
                    std::vector<Constraint> aRowOwn;
                    gatherDecidedConstraints(rDgm, *aRowNode->second, xRow, aRowOwn);
                    for (const Constraint& rConstraint : aRowOwn)
                        if (rConstraint.mnType == XML_w && rConstraint.mnFor == XML_ch
                            && rConstraint.msForName == aTargetName
                            && rConstraint.mnRefType == XML_w && rConstraint.msRefForName.isEmpty()
                            && rConstraint.mfFactor > 0.0)
                            fPart = rConstraint.mfFactor;
                    const AlgAtom* pRowAlg(algorithmOf(rDgm, *aRowNode->second, xRow));
                    const bool bFromRight(pRowAlg && pRowAlg->getMap().count(XML_linDir)
                                          && pRowAlg->getMap().find(XML_linDir)->second
                                                 == XML_fromR);
                    aTargetSize = awt::Size(
                        static_cast<sal_Int32>(aRow->second->getSize().Width * fPart),
                        aRow->second->getSize().Height);
                    aTargetAt = awt::Point(bFromRight ? aRowAt->X + aRow->second->getSize().Width
                                                            - aTargetSize.Width
                                                      : aRowAt->X,
                                           aRowAt->Y);
                }
            }
            if (pFrom && aTo != rShapes.end() && aFromAt && aTargetAt && aParentAt)
            {
                const sal_Int32 nBeginSite(rMap.count(XML_begPts) ? rMap.find(XML_begPts)->second
                                                                  : XML_auto);
                const awt::Point aStart(siteOn(nBeginSite, *aFromAt, pFrom->getSize()));
                // auto picks the middle of the target's edge that faces the start
                sal_Int32 nEndSite(rMap.count(XML_endPts) ? rMap.find(XML_endPts)->second
                                                          : XML_auto);
                if (!isEdgeSite(nEndSite))
                {
                    double fNearest(std::numeric_limits<double>::max());
                    for (const sal_Int32 nSite : { XML_midL, XML_midR, XML_tCtr, XML_bCtr })
                    {
                        const awt::Point aAt(siteOn(nSite, *aTargetAt, aTargetSize));
                        const double fFar(std::hypot(aAt.X - aStart.X, aAt.Y - aStart.Y));
                        if (fFar < fNearest)
                        {
                            fNearest = fFar;
                            nEndSite = nSite;
                        }
                    }
                }
                const awt::Point aStop(siteOn(nEndSite, *aTargetAt, aTargetSize));
                const double fDx(aStop.X - aStart.X), fDy(aStop.Y - aStart.Y);
                const double fWay(std::hypot(fDx, fDy));
                if (fWay >= 1.0)
                {
                    double fBeginPad(0.0), fEndPad(0.0);
                    bool bPadsStated(false);
                    std::vector<Constraint> aOwn;
                    gatherDecidedConstraints(rDgm, *aNode->second, xOwn, aOwn);
                    for (const Constraint& rConstraint : aOwn)
                    {
                        if (rConstraint.mnType != XML_begPad && rConstraint.mnType != XML_endPad)
                            continue;
                        bPadsStated = true;
                        if (rConstraint.mnRefType != XML_connDist)
                            continue;
                        if (rConstraint.mnType == XML_begPad)
                            fBeginPad = rConstraint.mfFactor;
                        else
                            fEndPad = rConstraint.mfFactor;
                    }
                    if (!bPadsStated)
                        fBeginPad = fEndPad = 0.235;
                    const double fDrawn(std::max(1.0, fWay * (1.0 - fBeginPad - fEndPad)));
                    const double fMiddle(fBeginPad * fWay + fDrawn / 2.0);
                    const double fMiddleX(aStart.X + fDx / fWay * fMiddle);
                    const double fMiddleY(aStart.Y + fDy / fWay * fMiddle);
                    const awt::Size aLine(static_cast<sal_Int32>(fDrawn), 0);
                    pShape->setSize(aLine);
                    pShape->setChildSize(aLine);
                    pShape->setPosition(awt::Point(
                        static_cast<sal_Int32>(fMiddleX - fDrawn / 2.0) - aParentAt->X,
                        static_cast<sal_Int32>(fMiddleY) - aParentAt->Y));
                    double fTurn(basegfx::rad2deg(std::atan2(fDy, fDx)));
                    if (fTurn < 0.0)
                        fTurn += 360.0;
                    pShape->setRotation(static_cast<sal_Int32>(fTurn * PER_DEGREE));
                    continue;
                }
            }
        }

        if (nRoute == XML_curve || nRoute == XML_longCurve)
        {
            const auto aDim = rMap.find(XML_dim);
            const sal_Int32 nBeginSite(rMap.count(XML_begPts) ? rMap.find(XML_begPts)->second : 0);
            const sal_Int32 nEndSite(rMap.count(XML_endPts) ? rMap.find(XML_endPts)->second : 0);

            // A curved connector with a body that names the node it ends at only, its diam a
            // part of a name of the layout's own that its composite states as a part of its
            // width, is a block arrow bent round a circle, the circular arrow preset, the way
            // the ring draws its transition. Continuous_Cycle with two nodes is a composite,
            // "userA for ch forName sibTrans refType w fact 1.07" and the transition's "diam
            // refType userA fact -1": the arrow's circle is half that diam, and it runs through
            // midR and midL of the first node, so its middle stands below the node's middle by
            // what a chord of the node's width leaves of the radius; the second node stands on
            // the far side of it. The band is stemThick of the diam thick, the head reaches
            // hArH of it from the middle line, the head is wArH of it long, the pads are parts
            // of the node's width, and the angles are as the ring takes them. The circle the
            // preset draws is 0.99 of the stated one there, 7627 for 7705 in units of 360 EMU,
            // and the composite's width is taken through the node, the stated part of it the
            // node is, since the node is what the composite scaled to fit.
            const bool bBody(aDim == rMap.end() || aDim->second == XML_2D);

            // A curved connector with a body that names both its ends, in a composite that
            // states its diam and its h as parts of its own width, is a block arrow bent round
            // the arc from the one end to the other: Segmented_Cycle's wedges each carry two
            // spacers of a millimetre at the two ends of the wedge's arc, and the arrow of the
            // transition after a wedge runs from the one to the other, "srcNode dummy1a",
            // "dstNode dummy1b", tL to tL, with the wedge's diam, 0.84 of the width. The
            // circle's middle lies on the right of the way from the one to the other, the
            // wedge's own middle, and the arrow runs clockwise from the start to the end; a
            // diam of less than nothing puts the middle on the left and runs it the other way
            // round. The head is at the end that endSty arr names, and where begSty arr names
            // the start instead the arrow runs from the end back to the start, the other way
            // round the same circle, as the right-to-left branch of that layout has it. The
            // head reaches the stated h from the middle line, the band is 0.6 of that thick and
            // the head 0.5 of it long, and the preset draws the circle at 0.99 of the stated
            // one: Segmented_Cycle's arrows are 14209 for a wedge of 12644 and an h of 1204, in
            // units of 360 EMU, adj1 5085 and adj5 5932, and Block_Cycle2D's arrows say the
            // same, 629 thick for an h of 1049, the head 520 long.
            if (bBody && !aSourceName.isEmpty() && !aTargetName.isEmpty()
                && isEdgeSite(nBeginSite) && isEdgeSite(nEndSite))
            {
                const OUString aParentId(presentationParentOf(rDgm, xOwn->msModelId));
                const rtl::Reference<svx::diagram::Point> xParent(
                    rDgm.getData()->getPointByModelID(aParentId));
                const auto aParentNode
                    = xParent.is() ? aNodes.find(
                          xParent->getPresentation().msPresentationLayoutName)
                                   : aNodes.end();
                const auto aParentShape = xParent.is() ? rShapes.find(xParent) : rShapes.end();
                // diam and h may be parts of the width of a shape the composite holds, as in
                // "diam for des forName connector1 refType w refFor ch refForName gear1 fact 1.15";
                // the name of that shape, empty for the composite
                double fDiamOfWidth(0.0);
                double fHeightOfWidth(0.0);
                OUString aDiamOf, aHeightOf;
                if (aParentNode != aNodes.end() && aParentNode->second
                    && aParentShape != rShapes.end())
                {
                    std::vector<Constraint> aParents;
                    gatherDecidedConstraints(rDgm, *aParentNode->second, xParent, aParents);
                    for (const Constraint& rParent : aParents)
                    {
                        if ((rParent.mnFor != XML_ch && rParent.mnFor != XML_des)
                            || rParent.mnRefType != XML_w
                            || rParent.msForName != aNode->second->getName()
                            || rParent.mfFactor == 0.0)
                            continue;
                        if (rParent.mnType == XML_diam)
                        {
                            fDiamOfWidth = rParent.mfFactor;
                            aDiamOf = rParent.msRefForName;
                        }
                        else if (rParent.mnType == XML_h && rParent.mfFactor > 0.0)
                        {
                            fHeightOfWidth = rParent.mfFactor;
                            aHeightOf = rParent.msRefForName;
                        }
                    }
                }
                // the width of the named shape nearest the arrow, the composite's own where none
                // is named or found
                const auto aWidthOfNamed
                    = [&rDgm, &rShapes, &xOwn](const OUString& rName, double fOwn) {
                          if (rName.isEmpty())
                              return fOwn;
                          rtl::Reference<svx::diagram::Point> xNamed(presentationNamedBeside(
                              rDgm, xOwn->msModelId, rName, /*bBefore*/ true));
                          if (!xNamed.is())
                              xNamed = presentationNamedBeside(rDgm, xOwn->msModelId, rName,
                                                               /*bBefore*/ false);
                          const auto aNamed = xNamed.is() ? rShapes.find(xNamed) : rShapes.end();
                          if (aNamed == rShapes.end() || !aNamed->second
                              || aNamed->second->getSize().Width <= 0)
                              return fOwn;
                          return static_cast<double>(aNamed->second->getSize().Width);
                      };
                std::vector<OUString> aEnds;
                if (fDiamOfWidth != 0.0 && fHeightOfWidth > 0.0)
                {
                    // both ends may stand before the arrow, Segmented_Cycle's spacers do, so
                    // each is looked for on the near side first and on the other side then
                    rtl::Reference<svx::diagram::Point> xFrom(presentationNamedBeside(
                        rDgm, xOwn->msModelId, aSourceName, /*bBefore*/ true));
                    if (!xFrom.is())
                        xFrom = presentationNamedBeside(rDgm, xOwn->msModelId, aSourceName,
                                                        /*bBefore*/ false);
                    rtl::Reference<svx::diagram::Point> xTo(presentationNamedBeside(
                        rDgm, xOwn->msModelId, aTargetName, /*bBefore*/ false));
                    if (!xTo.is())
                        xTo = presentationNamedBeside(rDgm, xOwn->msModelId, aTargetName,
                                                      /*bBefore*/ true);
                    if (xFrom.is() && xTo.is())
                    {
                        aEnds.push_back(xFrom->msModelId);
                        aEnds.push_back(xTo->msModelId);
                    }
                }
                const auto aBeginStyle = rMap.find(XML_begSty);
                const auto aEndStyle = rMap.find(XML_endSty);
                const bool bHeadAtStart(aBeginStyle != rMap.end() && aBeginStyle->second == XML_arr
                                        && (aEndStyle == rMap.end()
                                            || aEndStyle->second != XML_arr));
                const std::optional<awt::Point> aParentAt(absolutePlaceOf(rDgm, aParentId));
                if (aEnds.size() == 2 && aParentAt)
                {
                    const rtl::Reference<svx::diagram::Point> xFrom(
                        rDgm.getData()->getPointByModelID(aEnds[0]));
                    const rtl::Reference<svx::diagram::Point> xTo(
                        rDgm.getData()->getPointByModelID(aEnds[1]));
                    const auto aFrom = rShapes.find(xFrom);
                    const auto aTo = rShapes.find(xTo);
                    const std::optional<awt::Point> aFromAt(absolutePlaceOf(rDgm, aEnds[0]));
                    const std::optional<awt::Point> aToAt(absolutePlaceOf(rDgm, aEnds[1]));
                    if (aFrom != rShapes.end() && aTo != rShapes.end() && aFromAt && aToAt)
                    {
                        const awt::Point aFromSite(
                            siteOn(nBeginSite, *aFromAt, aFrom->second->getSize()));
                        const awt::Point aToSite(
                            siteOn(nEndSite, *aToAt, aTo->second->getSize()));
                        const awt::Point aStart(bHeadAtStart ? aToSite : aFromSite);
                        const awt::Point aStop(bHeadAtStart ? aFromSite : aToSite);
                        const double fDx(aStop.X - aStart.X), fDy(aStop.Y - aStart.Y);
                        const double fWay(std::hypot(fDx, fDy));
                        // a composite with an ar is as wide as that ratio of its height lets
                        // it be, Segmented_Cycle's square in a wider box, and its parts of w
                        // are parts of that
                        double fWidth(aParentShape->second->getSize().Width);
                        const AlgAtom* pParentAlg(
                            algorithmOf(rDgm, *aParentNode->second, xParent));
                        if (pParentAlg && pParentAlg->getAspectRatio() > 0.0)
                            fWidth = std::min(fWidth, aParentShape->second->getSize().Height
                                                          * pParentAlg->getAspectRatio());
                        const double fCircle(std::max(
                            fWay / 2.0,
                            std::abs(fDiamOfWidth) * aWidthOfNamed(aDiamOf, fWidth) / 2.0));
                        const double fHead(fHeightOfWidth * aWidthOfNamed(aHeightOf, fWidth));
                        if (fWay >= 1.0 && fHead > 0.0)
                        {
                            const double fOff(std::sqrt(
                                std::max(0.0, fCircle * fCircle - fWay * fWay / 4.0)));
                            // the middle stays where the way from the named start to the
                            // named end puts it, so run the other way it lies on the left
                            const bool bClockwise((fDiamOfWidth > 0.0) != bHeadAtStart);
                            const double fSide(bClockwise ? 1.0 : -1.0);
                            const double fMiddleX((aStart.X + aStop.X) / 2.0
                                                  + fSide * fOff * -fDy / fWay);
                            const double fMiddleY((aStart.Y + aStop.Y) / 2.0
                                                  + fSide * fOff * fDx / fWay);
                            double fBeginPad(0.0), fEndPad(0.0);
                            {
                                std::vector<Constraint> aOwn;
                                gatherDecidedConstraints(rDgm, *aNode->second, xOwn, aOwn);
                                for (const Constraint& rOwn : aOwn)
                                    if (rOwn.msForName.isEmpty()
                                        && rOwn.mnRefType == XML_connDist)
                                    {
                                        if (rOwn.mnType == XML_begPad)
                                            fBeginPad = rOwn.mfFactor;
                                        else if (rOwn.mnType == XML_endPad)
                                            fEndPad = rOwn.mfFactor;
                                    }
                            }
                            const double fStem(0.6 * fHead);
                            const double fHeadLength(0.5 * fHead);
                            const double fReach(fHead - fStem / 2.0);
                            const auto aAngleOf = [fMiddleX, fMiddleY](const awt::Point& rAt) {
                                return basegfx::rad2deg(
                                    std::atan2(rAt.Y - fMiddleY, rAt.X - fMiddleX));
                            };
                            const double fTurn(bClockwise ? 1.0 : -1.0);
                            double fStartAngle(aAngleOf(aStart)
                                               + fTurn
                                                     * basegfx::rad2deg(fBeginPad * fWay
                                                                        / fCircle));
                            double fEndAngle(aAngleOf(aStop)
                                             - fTurn
                                                   * basegfx::rad2deg(
                                                       (fEndPad * fWay + fHeadLength)
                                                       / fCircle));
                            fStartAngle = std::fmod(fStartAngle + 720.0, 360.0);
                            fEndAngle = std::fmod(fEndAngle + 720.0, 360.0);
                            const double fHeadAngle(basegfx::rad2deg(fHeadLength / fCircle));
                            const double fBox(2.0 * (0.99 * fCircle + fReach));
                            const awt::Size aSquare(static_cast<sal_Int32>(fBox),
                                                    static_cast<sal_Int32>(fBox));
                            pShape->setSize(aSquare);
                            pShape->setChildSize(aSquare);
                            pShape->setPosition(awt::Point(
                                static_cast<sal_Int32>(fMiddleX - fBox / 2.0) - aParentAt->X,
                                static_cast<sal_Int32>(fMiddleY - fBox / 2.0) - aParentAt->Y));
                            pShape->setRotation(0);
                            const sal_Int32 nPreset(bClockwise ? XML_circularArrow
                                                               : XML_leftCircularArrow);
                            pShape->setSubType(nPreset);
                            auto& rProperties(*pShape->getCustomShapeProperties());
                            rProperties.setShapePresetType(nPreset);
                            auto& rGuides(rProperties.getAdjustmentGuideList());
                            const auto aSet = [&rGuides](const OUString& rName, double fValue) {
                                setGuideValue(rGuides, rName,
                                              static_cast<sal_Int32>(std::lround(fValue)));
                            };
                            aSet(u"adj1"_ustr, fStem / fBox * 100000.0);
                            aSet(u"adj2"_ustr, fHeadAngle * 60000.0);
                            aSet(u"adj3"_ustr, fEndAngle * 60000.0);
                            aSet(u"adj4"_ustr, fStartAngle * 60000.0);
                            aSet(u"adj5"_ustr, fReach / fBox * 100000.0);
                            continue;
                        }
                    }
                }
            }

            // A curved connector with a body in a composite whose diam is a part of a name of
            // the layout's own that the composite states as a part of its width, Continuous_
            // Cycle with two nodes: "userA for ch forName sibTrans refType w fact 1.07" and the
            // transition's "diam refType userA fact -1". Its begin site is on the shape of the
            // node it comes from, or of srcNode where one is named, and its end site on the
            // shape of dstNode. The circle's middle is at half the diam from both sites, one of
            // the two points that are, and the sign of the diam says which: less than nothing
            // puts it on the left of the way from the begin to the end, more on the right. The
            // user's file Continuous_Cycle_2items_manu_begPts holds seven such arrows, ctr, midL,
            // midR and bCtr on node1 to midL of node2, the last two the same but for the sign,
            // and every middle is at 7704 from both sites in units of 360 EMU. The arrow then
            // runs from the begin to the end the long way round for connRout longCurve and the
            // short way for curve, which with the side of the middle settles the turn: the long
            // way round a middle on the left is clockwise, the circularArrow preset, the long way
            // round a middle on the right is the other way, leftCircularArrow. The head reaches
            // hArH of the diam from the middle line, the band is stemThick of it thick, the head
            // wArH of it long, the pads parts of the way between the two sites, and the
            // preset's circle 0.99 of the stated one, 7627 for 7705. The composite's width is
            // taken through the node, the stated part of it the node is, since the node is what
            // the composite scaled to fit.
            // a site may be the middle of the shape too, ctr, the first two of those seven
            const auto aIsSite
                = [](sal_Int32 nSite) { return isEdgeSite(nSite) || nSite == XML_ctr; };
            if (bBody && !aTargetName.isEmpty() && aIsSite(nBeginSite) && aIsSite(nEndSite))
            {
                std::vector<Constraint> aOwn;
                gatherDecidedConstraints(rDgm, *aNode->second, xOwn, aOwn);
                sal_Int32 nUserName(0);
                double fDiamPart(0.0);
                for (const Constraint& rOwn : aOwn)
                    if (rOwn.mnType == XML_diam && rOwn.msForName.isEmpty()
                        && isUserVariable(rOwn.mnRefType) && rOwn.mfFactor != 0.0)
                    {
                        nUserName = rOwn.mnRefType;
                        fDiamPart = rOwn.mfFactor;
                    }
                const OUString aParentId(presentationParentOf(rDgm, xOwn->msModelId));
                const rtl::Reference<svx::diagram::Point> xParent(
                    rDgm.getData()->getPointByModelID(aParentId));
                const auto aParentNode
                    = xParent.is() ? aNodes.find(
                          xParent->getPresentation().msPresentationLayoutName)
                                   : aNodes.end();
                double fUserOfWidth(0.0);
                std::map<OUString, double> aNodeOfWidth;
                if (nUserName && aParentNode != aNodes.end() && aParentNode->second)
                {
                    std::vector<Constraint> aParents;
                    gatherDecidedConstraints(rDgm, *aParentNode->second, xParent, aParents);
                    for (const Constraint& rParent : aParents)
                    {
                        if (rParent.mnFor != XML_ch || rParent.mnRefType != XML_w
                            || rParent.mfFactor <= 0.0)
                            continue;
                        if (rParent.mnType == nUserName
                            && rParent.msForName == aNode->second->getName())
                            fUserOfWidth = rParent.mfFactor;
                        else if (rParent.mnType == XML_w && !rParent.msForName.isEmpty())
                            aNodeOfWidth[rParent.msForName] = rParent.mfFactor;
                    }
                }
                const auto aBeside = [&rDgm, &xOwn](const OUString& rName) {
                    rtl::Reference<svx::diagram::Point> xFound(presentationNamedBeside(
                        rDgm, xOwn->msModelId, rName, /*bBefore*/ true));
                    if (!xFound.is())
                        xFound = presentationNamedBeside(rDgm, xOwn->msModelId, rName,
                                                         /*bBefore*/ false);
                    return xFound;
                };
                rtl::Reference<svx::diagram::Point> xFrom;
                rtl::Reference<svx::diagram::Point> xTo;
                if (fUserOfWidth > 0.0)
                {
                    xTo = aBeside(aTargetName);
                    if (!aSourceName.isEmpty())
                        xFrom = aBeside(aSourceName);
                    else
                    {
                        // the node this transition comes from is the one drawn before it among
                        // the composite's children, the nearest shape before it that is no
                        // connector
                        const std::vector<OUString> aSiblings(
                            presentationChildrenOf(rDgm, aParentId));
                        const auto aOwnAt
                            = std::find(aSiblings.begin(), aSiblings.end(), xOwn->msModelId);
                        for (auto aIt = aOwnAt; aIt != aSiblings.end() && aIt != aSiblings.begin();)
                        {
                            --aIt;
                            const rtl::Reference<svx::diagram::Point> xSibling(
                                rDgm.getData()->getPointByModelID(*aIt));
                            const auto aSibling = xSibling.is() ? rShapes.find(xSibling)
                                                                : rShapes.end();
                            if (aSibling != rShapes.end() && aSibling->second
                                && aSibling->second->getSubType() != XML_conn
                                && aSibling->second->getSize().Width > 0)
                            {
                                xFrom = xSibling;
                                break;
                            }
                        }
                    }
                }
                const auto aFrom = xFrom.is() ? rShapes.find(xFrom) : rShapes.end();
                const auto aTo = xTo.is() ? rShapes.find(xTo) : rShapes.end();
                const std::optional<awt::Point> aFromAt(
                    xFrom.is() ? absolutePlaceOf(rDgm, xFrom->msModelId) : std::nullopt);
                const std::optional<awt::Point> aToAt(
                    xTo.is() ? absolutePlaceOf(rDgm, xTo->msModelId) : std::nullopt);
                const std::optional<awt::Point> aParentAt(absolutePlaceOf(rDgm, aParentId));
                const auto aToPart = xTo.is() ? aNodeOfWidth.find(
                                         xTo->getPresentation().msPresentationLayoutName)
                                              : aNodeOfWidth.end();
                if (aFrom != rShapes.end() && aTo != rShapes.end() && aFromAt && aToAt
                    && aParentAt && aToPart != aNodeOfWidth.end()
                    && aTo->second->getSize().Width > 0)
                {
                    const awt::Size aNodeSize(aTo->second->getSize());
                    const double fDiam(std::abs(fDiamPart) * fUserOfWidth / aToPart->second
                                       * aNodeSize.Width);
                    double fStem(0.0), fHeadLength(0.0), fHeadReach(0.0), fBeginPad(0.0),
                        fEndPad(0.0);
                    for (const Constraint& rOwn : aOwn)
                    {
                        if (!rOwn.msForName.isEmpty())
                            continue;
                        if (rOwn.mnRefType == nUserName && rOwn.mfFactor > 0.0)
                        {
                            if (rOwn.mnType == XML_stemThick)
                                fStem = rOwn.mfFactor * fDiam;
                            else if (rOwn.mnType == XML_wArH)
                                fHeadLength = rOwn.mfFactor * fDiam;
                            else if (rOwn.mnType == XML_hArH)
                                fHeadReach = rOwn.mfFactor * fDiam;
                        }
                        else if (rOwn.mnRefType == XML_connDist)
                        {
                            if (rOwn.mnType == XML_begPad)
                                fBeginPad = rOwn.mfFactor;
                            else if (rOwn.mnType == XML_endPad)
                                fEndPad = rOwn.mfFactor;
                        }
                    }
                    const awt::Point aStart(
                        siteOn(nBeginSite, *aFromAt, aFrom->second->getSize()));
                    const awt::Point aStop(siteOn(nEndSite, *aToAt, aTo->second->getSize()));
                    const double fDx(aStop.X - aStart.X), fDy(aStop.Y - aStart.Y);
                    const double fWay(std::hypot(fDx, fDy));
                    const double fCircle(std::max(fWay / 2.0, fDiam / 2.0));
                    const double fReach(fHeadReach - fStem / 2.0);
                    if (fWay >= 1.0 && fStem > 0.0 && fReach > 0.0)
                    {
                        const bool bRight(fDiamPart > 0.0);
                        const bool bLong(nRoute == XML_longCurve);
                        const bool bClockwise(bRight != bLong);
                        const double fOff(
                            std::sqrt(std::max(0.0, fCircle * fCircle - fWay * fWay / 4.0)));
                        const double fSide(bRight ? 1.0 : -1.0);
                        const double fMiddleX((aStart.X + aStop.X) / 2.0
                                              + fSide * fOff * -fDy / fWay);
                        const double fMiddleY((aStart.Y + aStop.Y) / 2.0
                                              + fSide * fOff * fDx / fWay);
                        const auto aAngleOf = [fMiddleX, fMiddleY](const awt::Point& rAt) {
                            return basegfx::rad2deg(
                                std::atan2(rAt.Y - fMiddleY, rAt.X - fMiddleX));
                        };
                        const double fTurn(bClockwise ? 1.0 : -1.0);
                        double fStart(aAngleOf(aStart)
                                      + fTurn * basegfx::rad2deg(fBeginPad * fWay / fCircle));
                        double fEnd(aAngleOf(aStop)
                                    - fTurn
                                          * basegfx::rad2deg((fEndPad * fWay + fHeadLength)
                                                             / fCircle));
                        fStart = std::fmod(fStart + 720.0, 360.0);
                        fEnd = std::fmod(fEnd + 720.0, 360.0);
                        const double fHeadAngle(basegfx::rad2deg(fHeadLength / fCircle));
                        const double fBox(2.0 * (0.99 * fCircle + fReach));
                        const awt::Size aSquare(static_cast<sal_Int32>(fBox),
                                                static_cast<sal_Int32>(fBox));
                        pShape->setSize(aSquare);
                        pShape->setChildSize(aSquare);
                        pShape->setPosition(awt::Point(
                            static_cast<sal_Int32>(fMiddleX - fBox / 2.0) - aParentAt->X,
                            static_cast<sal_Int32>(fMiddleY - fBox / 2.0) - aParentAt->Y));
                        pShape->setRotation(0);
                        const sal_Int32 nPreset(bClockwise ? XML_circularArrow
                                                           : XML_leftCircularArrow);
                        pShape->setSubType(nPreset);
                        auto& rProperties(*pShape->getCustomShapeProperties());
                        rProperties.setShapePresetType(nPreset);
                        auto& rGuides(rProperties.getAdjustmentGuideList());
                        const auto aSet = [&rGuides](const OUString& rName, double fValue) {
                            setGuideValue(rGuides, rName,
                                          static_cast<sal_Int32>(std::lround(fValue)));
                        };
                        aSet(u"adj1"_ustr, fStem / fBox * 100000.0);
                        aSet(u"adj2"_ustr, fHeadAngle * 60000.0);
                        aSet(u"adj3"_ustr, fEnd * 60000.0);
                        aSet(u"adj4"_ustr, fStart * 60000.0);
                        aSet(u"adj5"_ustr, fReach / fBox * 100000.0);
                        continue;
                    }
                }
            }

            // a 2D one, a block arrow bent round, is the same circle where it names both ends
            const bool bLine(aDim != rMap.end() && aDim->second == XML_1D);
            if ((bLine || (!aSourceName.isEmpty() && !aTargetName.isEmpty()))
                && isEdgeSite(nBeginSite) && isEdgeSite(nEndSite))
            {
                rtl::Reference<svx::diagram::Point> xFrom;
                rtl::Reference<svx::diagram::Point> xTo;
                if (!aSourceName.isEmpty() && !aTargetName.isEmpty())
                {
                    xFrom = presentationNamedBeside(rDgm, xOwn->msModelId, aSourceName,
                                                    /*bBefore*/ true);
                    xTo = presentationNamedBeside(rDgm, xOwn->msModelId, aTargetName,
                                                  /*bBefore*/ false);
                }
                else
                {
                    // the shapes before and after the line among its row's children
                    const OUString aRow(presentationParentOf(rDgm, xOwn->msModelId));
                    const std::vector<OUString> aInRow(presentationChildrenOf(rDgm, aRow));
                    const auto aOwnAt = std::find(aInRow.begin(), aInRow.end(), xOwn->msModelId);
                    if (aOwnAt != aInRow.end() && aOwnAt != aInRow.begin()
                        && aOwnAt + 1 != aInRow.end())
                    {
                        xFrom = rDgm.getData()->getPointByModelID(*(aOwnAt - 1));
                        xTo = rDgm.getData()->getPointByModelID(*(aOwnAt + 1));
                    }
                }
                const auto aFrom = xFrom.is() ? rShapes.find(xFrom) : rShapes.end();
                const auto aTo = xTo.is() ? rShapes.find(xTo) : rShapes.end();
                const std::optional<awt::Point> aFromAt(
                    xFrom.is() ? absolutePlaceOf(rDgm, xFrom->msModelId) : std::nullopt);
                const std::optional<awt::Point> aToAt(
                    xTo.is() ? absolutePlaceOf(rDgm, xTo->msModelId) : std::nullopt);
                const std::optional<awt::Point> aParentAt(
                    absolutePlaceOf(rDgm, presentationParentOf(rDgm, xOwn->msModelId)));
                if (aFrom != rShapes.end() && aTo != rShapes.end() && aFromAt && aToAt
                    && aParentAt)
                {
                    const awt::Point aStart(siteOn(nBeginSite, *aFromAt, aFrom->second->getSize()));
                    const awt::Point aStop(siteOn(nEndSite, *aToAt, aTo->second->getSize()));
                    const double fDx(aStop.X - aStart.X), fDy(aStop.Y - aStart.Y);
                    const double fWay(std::hypot(fDx, fDy));
                    if (fWay >= 1.0)
                    {
                        double fPart(1.0);
                        std::vector<Constraint> aOwn;
                        gatherDecidedConstraints(rDgm, *aNode->second, xOwn, aOwn);
                        for (const Constraint& rConstraint : aOwn)
                            if (rConstraint.mnType == XML_diam && rConstraint.msForName.isEmpty()
                                && rConstraint.mnRefType == XML_connDist
                                && rConstraint.mfFactor != 0.0)
                                fPart = rConstraint.mfFactor;
                        const double fDiameter(std::max(fWay, std::abs(fPart) * fWay));
                        const double fRadius(fDiameter / 2.0);
                        const double fOff(
                            std::sqrt(std::max(0.0, fRadius * fRadius - fWay * fWay / 4.0)));
                        const double fSide(fPart < 0.0 ? -1.0 : 1.0);
                        const double fMiddleX((aStart.X + aStop.X) / 2.0
                                              + fSide * fOff * -fDy / fWay);
                        const double fMiddleY((aStart.Y + aStop.Y) / 2.0
                                              + fSide * fOff * fDx / fWay);
                        // the box holds the band round the circle too, as wide as a layout above
                        // states the transition, a part of a named shape's width
                        double fBand(0.0);
                        for (const LayoutNode* pUp = aNode->second->getParentLayoutNode();
                             pUp && fBand <= 0.0; pUp = pUp->getParentLayoutNode())
                        {
                            std::vector<Constraint> aAbove;
                            gatherOwnConstraints(*pUp, aAbove);
                            for (const Constraint& rConstraint : aAbove)
                            {
                                if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
                                    || rConstraint.mfFactor <= 0.0
                                    || rConstraint.msRefForName.isEmpty()
                                    || (rConstraint.msForName != aNode->second->getName()
                                        && !(rConstraint.msForName.isEmpty()
                                             && rConstraint.mnPointType == XML_sibTrans)))
                                    continue;
                                for (const auto& rEntry : rShapes)
                                    if (rEntry.second
                                        && rEntry.second->getInternalName()
                                               == rConstraint.msRefForName)
                                    {
                                        fBand = rConstraint.mfFactor
                                                * rEntry.second->getSize().Width;
                                        break;
                                    }
                                if (fBand > 0.0)
                                    break;
                            }
                        }
                        const double fBox(fDiameter + fBand);
                        const awt::Size aSquare(static_cast<sal_Int32>(fBox),
                                                static_cast<sal_Int32>(fBox));
                        pShape->setSize(aSquare);
                        pShape->setChildSize(aSquare);
                        pShape->setPosition(awt::Point(
                            static_cast<sal_Int32>(fMiddleX - fBox / 2.0) - aParentAt->X,
                            static_cast<sal_Int32>(fMiddleY - fBox / 2.0) - aParentAt->Y));
                        pShape->setRotation(0);
                        continue;
                    }
                }
            }
        }

        if (nRoute == XML_bend)
        {
            // A bend that names the shape it starts at and the one it ends at runs from the
            // bottom middle of the one to the top middle of the other, begPts bCtr and endPts
            // tCtr: Circle_Picture_Hierarchy's elbows start under the picture of a node, not
            // under the middle of its cell, the picture standing at the cell's left. The branch
            // drew the elbow from the cell's middle, and it is moved to the named shapes here.
            if (aSourceName.isEmpty() || aTargetName.isEmpty())
                continue;
            const auto aBegin = rMap.find(XML_begPts);
            const auto aEnd = rMap.find(XML_endPts);
            if (aBegin == rMap.end() || aEnd == rMap.end() || aBegin->second != XML_bCtr
                || aEnd->second != XML_tCtr)
                continue;
            const rtl::Reference<svx::diagram::Point> xSource(
                presentationNamedBeside(rDgm, xOwn->msModelId, aSourceName, /*bBefore*/ true));
            const rtl::Reference<svx::diagram::Point> xTarget(
                presentationNamedBeside(rDgm, xOwn->msModelId, aTargetName, /*bBefore*/ false));
            const auto aSource = xSource.is() ? rShapes.find(xSource) : rShapes.end();
            const auto aTarget = xTarget.is() ? rShapes.find(xTarget) : rShapes.end();
            if (aSource == rShapes.end() || aTarget == rShapes.end())
                continue;
            const std::optional<awt::Point> aSourceAt(absolutePlaceOf(rDgm, xSource->msModelId));
            const std::optional<awt::Point> aTargetAt(absolutePlaceOf(rDgm, xTarget->msModelId));
            const std::optional<awt::Point> aParentAt(
                absolutePlaceOf(rDgm, presentationParentOf(rDgm, xOwn->msModelId)));
            if (!aSourceAt || !aTargetAt || !aParentAt)
                continue;
            const awt::Size& rSourceSize(aSource->second->getSize());
            const awt::Size& rTargetSize(aTarget->second->getSize());
            const sal_Int32 nFromX(aSourceAt->X + rSourceSize.Width / 2);
            const sal_Int32 nFromY(aSourceAt->Y + rSourceSize.Height);
            const sal_Int32 nToX(aTargetAt->X + rTargetSize.Width / 2);
            const sal_Int32 nToY(aTargetAt->Y);
            if (nToY <= nFromY)
                continue;
            const sal_Int32 nWidth(std::max<sal_Int32>(91440, std::abs(nToX - nFromX)));
            const awt::Size aElbow(nWidth, nToY - nFromY);
            pShape->setSize(aElbow);
            pShape->setChildSize(aElbow);
            pShape->setPosition(awt::Point(
                std::min(nFromX, nToX) - (nWidth == 91440 ? 45720 : 0) - aParentAt->X,
                nFromY - aParentAt->Y));
            // own copy of the custom shape properties, the mirroring is kept there
            pShape->getCustomShapeProperties()
                = std::make_shared<CustomShapeProperties>(*pShape->getCustomShapeProperties());
            pShape->setFlip(nToX < nFromX, false);
            continue;
        }

        // A connector between two siblings of a row leaves a part of its way free at each end,
        // begPad and endPad as parts of connDist, and connDist is the way between the shapes of
        // the two nodes it joins, not the room the row gave the connector: Accent_Process's
        // arrows have a slot of a third of a composite, and run from the parent's box of the
        // one, 0.83 of its composite wide, to the parent's box of the next, 0.17 of a composite
        // further than the slot. The arrow is drawn over that way less the two pads; its h, a
        // part of the slot, was read when the connector was laid out. The shapes of a node are
        // the leaves that present it, the composite around them presents it as well and is left
        // out. Everything is laid out by now, so both edges are known.
        // only a connector in a row, the ring's connectors keep the ring's places
        const rtl::Reference<svx::diagram::Point> xRowPoint(
            rDgm.getData()->getPointByModelID(presentationParentOf(rDgm, xOwn->msModelId)));
        const auto aRowNode
            = xRowPoint.is()
                  ? aNodes.find(xRowPoint->getPresentation().msPresentationLayoutName)
                  : aNodes.end();
        const AlgAtom* pRowAlg(aRowNode != aNodes.end() && aRowNode->second
                                   ? algorithmOf(rDgm, *aRowNode->second, xRowPoint)
                                   : nullptr);
        double fBefore(0.0), fAfter(0.0);
        // A pad stated as a part of the connector's own w or h is a length, not a part of the
        // way: Process_List's sibTrans leaves 0.25 of its w free at each end, its w being its
        // h, the gap between two boxes, so the arrow is half the gap, 80449 of 160898.
        sal_Int32 nBeforeLength(0), nAfterLength(0);
        // The arrowhead's height, hArH, stated as a part of wArH, which is a part of the
        // connector's h or w: the arrow is that thick across its way. Process_List's arrows are
        // 80449 square, wArH a quarter of h and hArH twice that.
        double fArrowWidth(0.0), fArrowHeight(0.0);
        // a begPad or endPad stated with nothing on it is a pad of nothing, not one unstated
        bool bPadsStated(false);
        {
            const awt::Size aOwnSize(pShape->getSize());
            std::function<void(const LayoutAtom&)> aReadPads
                = [&aReadPads, &fBefore, &fAfter, &bPadsStated, &nBeforeLength, &nAfterLength,
                   &fArrowWidth, &fArrowHeight, &aOwnSize](const LayoutAtom& rAtom) {
                      for (const LayoutAtomPtr& pChild : rAtom.getChildren())
                      {
                          if (dynamic_cast<const LayoutNode*>(pChild.get()))
                              continue;
                          const ConstraintAtom* pConstraint
                              = dynamic_cast<const ConstraintAtom*>(pChild.get());
                          if (!pConstraint)
                          {
                              aReadPads(*pChild);
                              continue;
                          }
                          const Constraint& rConstraint(pConstraint->getConstraint());
                          if (rConstraint.mnType == XML_begPad || rConstraint.mnType == XML_endPad)
                              bPadsStated = true;
                          if (rConstraint.mfFactor <= 0.0)
                              continue;
                          const sal_Int32 nOwn(rConstraint.mnRefType == XML_w   ? aOwnSize.Width
                                               : rConstraint.mnRefType == XML_h ? aOwnSize.Height
                                                                                : 0);
                          if (rConstraint.mnType == XML_wArH)
                          {
                              if (nOwn > 0)
                                  fArrowWidth = nOwn * rConstraint.mfFactor;
                          }
                          else if (rConstraint.mnType == XML_hArH)
                          {
                              if (rConstraint.mnRefType == XML_wArH && fArrowWidth > 0.0)
                                  fArrowHeight = fArrowWidth * rConstraint.mfFactor;
                              else if (nOwn > 0)
                                  fArrowHeight = nOwn * rConstraint.mfFactor;
                          }
                          else if (rConstraint.mnRefType == XML_connDist)
                          {
                              if (rConstraint.mnType == XML_begPad)
                                  fBefore = rConstraint.mfFactor;
                              else if (rConstraint.mnType == XML_endPad)
                                  fAfter = rConstraint.mfFactor;
                          }
                          else if (nOwn > 0)
                          {
                              if (rConstraint.mnType == XML_begPad)
                                  nBeforeLength = static_cast<sal_Int32>(nOwn * rConstraint.mfFactor);
                              else if (rConstraint.mnType == XML_endPad)
                                  nAfterLength = static_cast<sal_Int32>(nOwn * rConstraint.mfFactor);
                          }
                      }
                  };
            aReadPads(*aNode->second);
        }
        // A connector that states no pads leaves 0.47 of the way free, Basic_Bending_Process's
        // arrows are 717692 in a slot of 1354138 and Circular_Bending_Process's triangles the
        // same part: 0.25 at the one end and 0.22 at the other, but which end is which differs
        // between the two files, so both get 0.235 and the arrow stands in the middle of the
        // way.
        if (pAlg->getType() == XML_conn && !bPadsStated)
            fBefore = fAfter = 0.235;
        if (pAlg->getType() == XML_conn && aSourceName.isEmpty() && aTargetName.isEmpty()
            && pRowAlg && (pRowAlg->getType() == XML_lin || pRowAlg->getType() == XML_snake))
        {
            // the way the arrow points on the page: its preset's way, turned by its rotation
            const sal_Int32 nArrow(pShape->getSubType());
            const bool bLyingPreset(nArrow == XML_rightArrow || nArrow == XML_leftArrow);
            const bool bStandingPreset(nArrow == XML_downArrow || nArrow == XML_upArrow);
            const sal_Int32 nTurn(((pShape->getRotation() / PER_DEGREE) % 360 + 360) % 360);
            const bool bQuarter(std::abs(nTurn - 90) <= 1 || std::abs(nTurn - 270) <= 1);
            const bool bHalf(std::abs(nTurn - 180) <= 1);
            const bool bPresetForward(nArrow == XML_rightArrow || nArrow == XML_downArrow);
            const bool bAlongX((bLyingPreset && !bQuarter) || (bStandingPreset && bQuarter));
            const bool bAlongY((bStandingPreset && !bQuarter) || (bLyingPreset && bQuarter));
            // a right arrow turned by 90 points down, by 270 up; a down arrow by 90 left
            const bool bForward(
                !bQuarter ? (bPresetForward != bHalf)
                          : (bLyingPreset ? (bPresetForward == (std::abs(nTurn - 90) <= 1))
                                          : (bPresetForward != (std::abs(nTurn - 90) <= 1))));
            if (((fBefore + fAfter > 0.0 && fBefore + fAfter < 1.0)
                 || nBeforeLength + nAfterLength > 0 || fArrowHeight > 0.0)
                && (bAlongX || bAlongY))
            {
                // The transition hangs on the parOf of the node it follows, as that
                // connection's sibTrans; the node after it is the next child of the same parent.
                const OUString aTransition(xOwn->getPresentation().msPresentationAssociationId);
                OUString aBefore, aNext, aParent;
                sal_Int32 nOrder(0);
                for (const rtl::Reference<svx::diagram::Connection>& rConnection :
                         rDgm.getData()->getConnections())
                    if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
                        && rConnection->msSibTransId == aTransition)
                    {
                        aBefore = rConnection->msDestId;
                        aParent = rConnection->msSourceId;
                        nOrder = rConnection->mnSourceOrder;
                    }
                for (const rtl::Reference<svx::diagram::Connection>& rConnection :
                         rDgm.getData()->getConnections())
                    if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
                        && rConnection->msSourceId == aParent
                        && rConnection->mnSourceOrder == nOrder + 1)
                        aNext = rConnection->msDestId;
                std::optional<sal_Int32> oFrom;
                std::optional<sal_Int32> oTo;
                for (const auto& rLeafEntry : rShapes)
                {
                    const ShapePtr& pLeaf(rLeafEntry.second);
                    if (!rLeafEntry.first.is() || !pLeaf || !pLeaf->getChildren().empty()
                        || pLeaf->getSize().Width <= 0 || pLeaf->getSize().Height <= 0)
                        continue;
                    const OUString& rOf(
                        rLeafEntry.first->getPresentation().msPresentationAssociationId);
                    if (rOf != aBefore && rOf != aNext)
                        continue;
                    const std::optional<awt::Point> aAt(
                        absolutePlaceOf(rDgm, rLeafEntry.first->msModelId));
                    if (!aAt)
                        continue;
                    const sal_Int32 nNear(bAlongX ? aAt->X : aAt->Y);
                    const sal_Int32 nFar(
                        nNear + (bAlongX ? pLeaf->getSize().Width : pLeaf->getSize().Height));
                    if ((rOf == aBefore) == bForward)
                        oFrom = oFrom ? std::max(*oFrom, nFar) : nFar;
                    else
                        oTo = oTo ? std::min(*oTo, nNear) : nNear;
                }
                const std::optional<awt::Point> aParentAt(
                    absolutePlaceOf(rDgm, presentationParentOf(rDgm, xOwn->msModelId)));
                awt::Point aAt(pShape->getPosition());
                awt::Size aSize(pShape->getSize());
                // a quarter turn has the stored box lying across the way, its sides swap
                // about its middle
                if (bQuarter)
                {
                    aAt = awt::Point(aAt.X + (aSize.Width - aSize.Height) / 2,
                                     aAt.Y + (aSize.Height - aSize.Width) / 2);
                    aSize = awt::Size(aSize.Height, aSize.Width);
                }
                sal_Int32 nStart(bAlongX ? aAt.X : aAt.Y);
                sal_Int32 nWay(bAlongX ? aSize.Width : aSize.Height);
                if (oFrom && oTo && aParentAt && *oTo > *oFrom)
                {
                    nStart = *oFrom - (bAlongX ? aParentAt->X : aParentAt->Y);
                    nWay = *oTo - *oFrom;
                }
                const sal_Int32 nDrawn(std::max<sal_Int32>(
                    1, static_cast<sal_Int32>(nWay * (1.0 - fBefore - fAfter)) - nBeforeLength
                           - nAfterLength));
                const sal_Int32 nOffset(
                    static_cast<sal_Int32>(nWay * (bForward ? fBefore : fAfter))
                    + (bForward ? nBeforeLength : nAfterLength));
                if (bAlongX)
                {
                    aSize.Width = nDrawn;
                    aAt.X = nStart + nOffset;
                    if (fArrowHeight > 0.0 && fArrowHeight < aSize.Height)
                    {
                        aAt.Y += (aSize.Height - static_cast<sal_Int32>(fArrowHeight)) / 2;
                        aSize.Height = static_cast<sal_Int32>(fArrowHeight);
                    }
                }
                else
                {
                    aSize.Height = nDrawn;
                    aAt.Y = nStart + nOffset;
                    if (fArrowHeight > 0.0 && fArrowHeight < aSize.Width)
                    {
                        aAt.X += (aSize.Width - static_cast<sal_Int32>(fArrowHeight)) / 2;
                        aSize.Width = static_cast<sal_Int32>(fArrowHeight);
                    }
                }
                // back to the stored box, turned about the same middle
                if (bQuarter)
                {
                    aAt = awt::Point(aAt.X + (aSize.Width - aSize.Height) / 2,
                                     aAt.Y + (aSize.Height - aSize.Width) / 2);
                    aSize = awt::Size(aSize.Height, aSize.Width);
                }
                pShape->setSize(aSize);
                pShape->setChildSize(aSize);
                pShape->setPosition(aAt);
                continue;
            }
        }

        // The ring itself, drawn as an arc round the nodes, stays as large and where the ring is:
        // the two shapes it names are on the ring, not at its ends.
        if (spansItsRing(rDgm, aNodes, xOwn))
            continue;

        // A connector that names no shapes but does name the sites it runs between, the middle
        // of the right edge to the middle of the left edge and the like, is the straight line
        // between those two sites. What it joins is what stands beside the branch it is in,
        // the shape before the group that holds it, and the shape that follows it. The line
        // lies flat before it is turned, its width the length of the way, and it is turned to
        // run from the one site to the other.
        const sal_Int32 nBegin(rMap.count(XML_begPts) ? rMap.find(XML_begPts)->second : 0);
        const sal_Int32 nEnd(rMap.count(XML_endPts) ? rMap.find(XML_endPts)->second : 0);
        if (aSourceName.isEmpty() && aTargetName.isEmpty() && isEdgeSite(nBegin)
            && isEdgeSite(nEnd))
        {
            const OUString aGroup(presentationParentOf(rDgm, xOwn->msModelId));
            const OUString aAbove(presentationParentOf(rDgm, aGroup));
            const std::vector<OUString> aOfAbove(presentationChildrenOf(rDgm, aAbove));
            const std::vector<OUString> aOfGroup(presentationChildrenOf(rDgm, aGroup));
            const auto aGroupAt = std::find(aOfAbove.begin(), aOfAbove.end(), aGroup);
            const auto aOwnAt = std::find(aOfGroup.begin(), aOfGroup.end(), xOwn->msModelId);
            if (aGroupAt == aOfAbove.end() || aGroupAt == aOfAbove.begin()
                || aOwnAt == aOfGroup.end() || aOwnAt + 1 == aOfGroup.end())
                continue;

            const rtl::Reference<svx::diagram::Point> xFrom(
                rDgm.getData()->getPointByModelID(*(aGroupAt - 1)));
            const rtl::Reference<svx::diagram::Point> xTo(
                rDgm.getData()->getPointByModelID(*(aOwnAt + 1)));
            const auto aFrom = xFrom.is() ? rShapes.find(xFrom) : rShapes.end();
            const auto aTo = xTo.is() ? rShapes.find(xTo) : rShapes.end();
            if (aFrom == rShapes.end() || aTo == rShapes.end())
                continue;

            const std::optional<awt::Point> aFromAt(absolutePlaceOf(rDgm, xFrom->msModelId));
            const std::optional<awt::Point> aToAt(absolutePlaceOf(rDgm, xTo->msModelId));
            const std::optional<awt::Point> aGroupPlace(absolutePlaceOf(rDgm, aGroup));
            if (!aFromAt || !aToAt || !aGroupPlace)
                continue;

            const awt::Point aStart(siteOn(nBegin, *aFromAt, aFrom->second->getSize()));
            const awt::Point aStop(siteOn(nEnd, *aToAt, aTo->second->getSize()));
            const double fDx(aStop.X - aStart.X), fDy(aStop.Y - aStart.Y);
            const double fWay(std::hypot(fDx, fDy));
            if (fWay < 1.0)
                continue;

            const sal_Int32 nThinnest(static_cast<sal_Int32>(fWay / 20));
            const sal_Int32 nThick(std::max<sal_Int32>(
                1, std::min<sal_Int32>(pShape->getSize().Height, nThinnest)));
            const awt::Size aLine(static_cast<sal_Int32>(fWay), nThick);
            pShape->setSize(aLine);
            pShape->setChildSize(aLine);
            pShape->setPosition(
                awt::Point((aStart.X + aStop.X) / 2 - aGroupPlace->X - aLine.Width / 2,
                           (aStart.Y + aStop.Y) / 2 - aGroupPlace->Y - aLine.Height / 2));
            pShape->setRotation(
                static_cast<sal_Int32>(basegfx::rad2deg(atan2(fDy, fDx)) * PER_DEGREE));
            continue;
        }

        if (aSourceName.isEmpty() || aTargetName.isEmpty())
            continue;

        const rtl::Reference<svx::diagram::Point> xSource(
            presentationNamedBeside(rDgm, xOwn->msModelId, aSourceName, /*bBefore*/ true));
        const rtl::Reference<svx::diagram::Point> xTarget(
            presentationNamedBeside(rDgm, xOwn->msModelId, aTargetName, /*bBefore*/ false));
        const auto aSource = xSource.is() ? rShapes.find(xSource) : rShapes.end();
        const auto aTarget = xTarget.is() ? rShapes.find(xTarget) : rShapes.end();
        if (aSource == rShapes.end() || aTarget == rShapes.end())
            continue;

        const std::optional<awt::Point> aSourceAt(absolutePlaceOf(rDgm, xSource->msModelId));
        const std::optional<awt::Point> aTargetAt(absolutePlaceOf(rDgm, xTarget->msModelId));
        const std::optional<awt::Point> aParentAt(
            absolutePlaceOf(rDgm, presentationParentOf(rDgm, xOwn->msModelId)));
        if (!aSourceAt || !aTargetAt || !aParentAt)
            continue;

        const awt::Size& rSourceSize(aSource->second->getSize());
        const awt::Size& rTargetSize(aTarget->second->getSize());
        const awt::Point aSourceMiddle(aSourceAt->X + rSourceSize.Width / 2,
                                       aSourceAt->Y + rSourceSize.Height / 2);
        const awt::Point aTargetMiddle(aTargetAt->X + rTargetSize.Width / 2,
                                       aTargetAt->Y + rTargetSize.Height / 2);
        const sal_Int32 nAcrossX(std::abs(aTargetMiddle.X - aSourceMiddle.X));
        const sal_Int32 nAcrossY(std::abs(aTargetMiddle.Y - aSourceMiddle.Y));

        // Two points to join, the dummy points of a process's nodes, stated as "w val=1" and
        // "h val=1", a millimetre each: the connector is the straight line from the middle of the
        // one to the middle of the other, as thick as it is, and turned to run between them at
        // whatever angle that is, round a corner as well.
        const sal_Int32 nPoint(o3tl::convert(1, o3tl::Length::mm, o3tl::Length::emu));
        if (rSourceSize.Width <= nPoint && rSourceSize.Height <= nPoint
            && rTargetSize.Width <= nPoint && rTargetSize.Height <= nPoint)
        {
            const double fDx(aTargetMiddle.X - aSourceMiddle.X);
            const double fDy(aTargetMiddle.Y - aSourceMiddle.Y);
            const double fWay(std::hypot(fDx, fDy));
            if (fWay < 1.0)
                continue;
            const awt::Size aLine(static_cast<sal_Int32>(fWay),
                                  std::max<sal_Int32>(1, pShape->getSize().Height));
            pShape->setSize(aLine);
            pShape->setChildSize(aLine);
            pShape->setPosition(
                awt::Point((aSourceMiddle.X + aTargetMiddle.X) / 2 - aParentAt->X - aLine.Width / 2,
                           (aSourceMiddle.Y + aTargetMiddle.Y) / 2 - aParentAt->Y
                               - aLine.Height / 2));
            pShape->setRotation(
                static_cast<sal_Int32>(basegfx::rad2deg(atan2(fDy, fDx)) * PER_DEGREE));
            continue;
        }

        // A line between two shapes that stand apart at an angle, the spokes of Radial_List
        // from the hidden circle in the middle to each node, runs straight from the edge of the
        // one to the edge of the other along the way between their middles. The edge of an
        // ellipse is where that way leaves it, of anything else its box.
        const auto aDim = rMap.find(XML_dim);
        const bool bLine(aDim != rMap.end() && aDim->second == XML_1D);
        const bool bInOneLine(nAcrossX >= nAcrossY
                                  ? nAcrossY * 4 <= std::min(rSourceSize.Height, rTargetSize.Height)
                                  : nAcrossX * 4 <= std::min(rSourceSize.Width, rTargetSize.Width));
        if (bLine && !bInOneLine)
        {
            const double fDx(aTargetMiddle.X - aSourceMiddle.X);
            const double fDy(aTargetMiddle.Y - aSourceMiddle.Y);
            const double fWay(std::hypot(fDx, fDy));
            if (fWay < 1.0)
                continue;
            const double fCos(fDx / fWay), fSin(fDy / fWay);
            const auto aReach = [fCos, fSin](const ShapePtr& pOf, const awt::Size& rSize) {
                const double fHalfWidth(rSize.Width / 2.0), fHalfHeight(rSize.Height / 2.0);
                if (pOf->getCustomShapeProperties()->getShapePresetType() == XML_ellipse)
                    return fHalfWidth * fHalfHeight
                           / std::hypot(fHalfHeight * fCos, fHalfWidth * fSin);
                double fToEdge(std::numeric_limits<double>::max());
                if (std::abs(fCos) > 1e-9)
                    fToEdge = std::min(fToEdge, fHalfWidth / std::abs(fCos));
                if (std::abs(fSin) > 1e-9)
                    fToEdge = std::min(fToEdge, fHalfHeight / std::abs(fSin));
                return fToEdge;
            };
            const double fFrom(aReach(aSource->second, rSourceSize));
            const double fTo(aReach(aTarget->second, rTargetSize));
            const double fLength(fWay - fFrom - fTo);
            if (fLength < 1.0)
                continue;
            const awt::Size aLine(static_cast<sal_Int32>(fLength),
                                  std::max<sal_Int32>(1, pShape->getSize().Height));
            const double fMiddleX(aSourceMiddle.X + fCos * (fFrom + fLength / 2.0));
            const double fMiddleY(aSourceMiddle.Y + fSin * (fFrom + fLength / 2.0));
            pShape->setSize(aLine);
            pShape->setChildSize(aLine);
            pShape->setPosition(
                awt::Point(static_cast<sal_Int32>(fMiddleX) - aParentAt->X - aLine.Width / 2,
                           static_cast<sal_Int32>(fMiddleY) - aParentAt->Y - aLine.Height / 2));
            pShape->setRotation(
                static_cast<sal_Int32>(basegfx::rad2deg(atan2(fDy, fDx)) * PER_DEGREE));
            continue;
        }

        // The flow runs the way the two lie apart, and this is for a connector between two that
        // stand in one line. One that goes round a corner is left alone.
        // A named connector that leaves a part of its way free at each end, begPad and endPad
        // as parts of connDist, runs over the way between the two shapes it names less those:
        // Accent_Process's arrows name the parent text on both sides, and connDist is from the
        // one text's right edge to the next one's left, 0.17 of a composite more than the slot.
        // A connector turned by a quarter, a triangle that points up in its box and is turned
        // by 90 to point along the flow, has its way on the box's height and its thickness on
        // the width; one turned by nothing or by half the other way round. The box is what
        // stands here, the turn is applied about its middle.
        awt::Point aOwnPos(pShape->getPosition());
        awt::Size aOwnSize(pShape->getSize());
        const bool bPads(fBefore + fAfter > 0.0 && fBefore + fAfter < 1.0);
        const sal_Int32 nQuarters(((pShape->getRotation() / PER_DEGREE) % 360 + 360) % 360);
        const bool bTurned(std::abs(nQuarters - 90) <= 1 || std::abs(nQuarters - 270) <= 1);
        const bool bHorizontal(nAcrossX >= nAcrossY);
        if (bHorizontal)
        {
            if (nAcrossY * 4 > std::min(rSourceSize.Height, rTargetSize.Height))
                continue;
            const bool bForward(aTargetMiddle.X > aSourceMiddle.X);
            const sal_Int32 nFrom(bForward ? aSourceAt->X + rSourceSize.Width
                                           : aTargetAt->X + rTargetSize.Width);
            const sal_Int32 nTo(bForward ? aTargetAt->X : aSourceAt->X);
            sal_Int32& rAlong(bTurned ? aOwnSize.Height : aOwnSize.Width);
            sal_Int32 nMiddleX((nFrom + nTo) / 2);
            // a line without pads runs the whole way
            if (bLine && !bPads && nTo > nFrom)
                rAlong = nTo - nFrom;
            if (bPads && nTo > nFrom)
            {
                const sal_Int32 nWay(nTo - nFrom);
                rAlong = static_cast<sal_Int32>(nWay * (1.0 - fBefore - fAfter));
                nMiddleX = nFrom + static_cast<sal_Int32>(nWay * (bForward ? fBefore : fAfter))
                           + rAlong / 2;
            }
            aOwnPos.X = nMiddleX - aParentAt->X - aOwnSize.Width / 2;
            aOwnPos.Y = (aSourceMiddle.Y + aTargetMiddle.Y) / 2 - aParentAt->Y
                        - aOwnSize.Height / 2;
        }
        else
        {
            if (nAcrossX * 4 > std::min(rSourceSize.Width, rTargetSize.Width))
                continue;
            const bool bForward(aTargetMiddle.Y > aSourceMiddle.Y);
            const sal_Int32 nFrom(bForward ? aSourceAt->Y + rSourceSize.Height
                                           : aTargetAt->Y + rTargetSize.Height);
            const sal_Int32 nTo(bForward ? aTargetAt->Y : aSourceAt->Y);
            sal_Int32& rAlong(bTurned ? aOwnSize.Width : aOwnSize.Height);
            sal_Int32 nMiddleY((nFrom + nTo) / 2);
            if (bLine && !bPads && nTo > nFrom)
                rAlong = nTo - nFrom;
            if (bPads && nTo > nFrom)
            {
                const sal_Int32 nWay(nTo - nFrom);
                rAlong = static_cast<sal_Int32>(nWay * (1.0 - fBefore - fAfter));
                nMiddleY = nFrom + static_cast<sal_Int32>(nWay * (bForward ? fBefore : fAfter))
                           + rAlong / 2;
            }
            aOwnPos.X = (aSourceMiddle.X + aTargetMiddle.X) / 2 - aParentAt->X
                        - aOwnSize.Width / 2;
            aOwnPos.Y = nMiddleY - aParentAt->Y - aOwnSize.Height / 2;
        }
        pShape->setSize(aOwnSize);
        pShape->setChildSize(aOwnSize);
        pShape->setPosition(aOwnPos);
        if (bPads
            && (aSharingNames.count(pShape->getInternalName())
                || (bSharingTransitions && pShape->getDataNodeType() == XML_sibTrans)))
            aSharingConnDist[pShape->getInternalName()].push_back(
                { pShape, bHorizontal, bTurned });
    }

    for (const auto& rEntry : aSharingConnDist)
    {
        const auto aAlong = [](const SettledConnector& rOne) {
            return (rOne.bHorizontal != rOne.bTurned) ? rOne.pShape->getSize().Width
                                                       : rOne.pShape->getSize().Height;
        };
        sal_Int32 nShortest(std::numeric_limits<sal_Int32>::max());
        for (const SettledConnector& rOne : rEntry.second)
            nShortest = std::min(nShortest, aAlong(rOne));
        if (nShortest <= 0)
            continue;
        for (const SettledConnector& rOne : rEntry.second)
        {
            awt::Size aSize(rOne.pShape->getSize());
            awt::Point aAt(rOne.pShape->getPosition());
            sal_Int32& rSide((rOne.bHorizontal != rOne.bTurned) ? aSize.Width : aSize.Height);
            if (rSide <= nShortest)
                continue;
            // the connector stays where its middle is
            if (rOne.bHorizontal != rOne.bTurned)
                aAt.X += (rSide - nShortest) / 2;
            else
                aAt.Y += (rSide - nShortest) / 2;
            rSide = nShortest;
            rOne.pShape->setSize(aSize);
            rOne.pShape->setChildSize(aSize);
            rOne.pShape->setPosition(aAt);
        }
    }
}

sal_Int32 ConditionAtom::getNodeCount(const SmartArtDiagram& rDgm,
                                      const OUString& rNodeId) const
{
    // Without an axis there is nothing to walk, and what the file asks for is the children.
    if (maIter.maAxis.empty())
        return static_cast<sal_Int32>(stepOfAxis(rDgm, rNodeId, XML_ch, XML_all).size());

    return static_cast<sal_Int32>(
        pointsAlongAxis(rDgm, maIter, rNodeId, /*bLastStepWhole*/ false).size());
}

void ConditionAtom::getNodePlace(const SmartArtDiagram& rDgm, const OUString& rNodeId,
                                 sal_Int32& rPosition, sal_Int32& rSiblings)
{
    rPosition = 0;
    rSiblings = 0;

    const OUString sNodeId(rNodeId);
    if (sNodeId.isEmpty())
        return;

    // A node hangs off its parent on a parOf, which carries the place it takes. What separates
    // it from the next one hangs off the same parOf, and takes the place of the node it follows.
    // The transitions between the nodes are one fewer than the nodes: the third of four nodes
    // has the last transition, and Circular_Bending_Process asks "revPos equ 1" on it to run
    // that one to the last node.
    OUString sParentId;
    sal_Int32 nOrder(0);
    bool bBetweenSiblings(false);
    for (const rtl::Reference<svx::diagram::Connection>& aCxn : rDgm.getData()->getConnections())
    {
        if (aCxn->mnXMLType != svx::diagram::TypeConstant::XML_parOf)
            continue;

        if (aCxn->msDestId == sNodeId || aCxn->msSibTransId == sNodeId
            || aCxn->msParTransId == sNodeId)
        {
            sParentId = aCxn->msSourceId;
            nOrder = aCxn->mnSourceOrder;
            bBetweenSiblings = aCxn->msSibTransId == sNodeId;
            break;
        }
    }

    if (sParentId.isEmpty())
        return;

    for (const rtl::Reference<svx::diagram::Connection>& aCxn : rDgm.getData()->getConnections())
        if (aCxn->mnXMLType == svx::diagram::TypeConstant::XML_parOf
            && aCxn->msSourceId == sParentId)
            rSiblings++;
    if (bBetweenSiblings && rSiblings > 1)
        rSiblings--;

    rPosition = nOrder + 1;
}

bool ConditionAtom::getDecision(const SmartArtDiagram& rDgm,
                                const rtl::Reference<svx::diagram::Point>& rPresPoint,
                                const OUString& rPassNodeId) const
{
    if (mIsElse)
        return true;
    if (!rPresPoint.is())
        return false;

    // A condition asks about the Point the pass of the loop stands on. Where the loop knows it,
    // that is what to ask about: a choose straight inside a for-each is met before the Point is
    // taken up, so rPresPoint still names what the loop runs over and answers for the wrong one.
    const OUString aAsk(!rPassNodeId.isEmpty()
                            ? rPassNodeId
                            : rPresPoint->getPresentation().msPresentationAssociationId);

    switch (maCond.mnFunc)
    {
    case XML_var:
    {
        if (maCond.mnArg == XML_dir)
        {
            // The direction is stated once, on the presentation Point of the root, and turns
            // the whole Diagram: a condition in a node below asks its own Point, which holds
            // norm, the default, so the nearest Point above it that states rev is the answer.
            // Basic_Timeline_RTL and StepDown_RTL state rev on the root only and ask in
            // "arrow" and "points", in "composite".
            sal_Int32 nDirection(rPresPoint->getLayoutVariables().mnDirection);
            OUString aAbove(rPresPoint->msModelId);
            while (nDirection == XML_norm && !aAbove.isEmpty())
            {
                aAbove = navigate(rDgm, svx::diagram::TypeConstant::XML_presParOf, aAbove,
                                  /*bSourceToDestination*/ false);
                const rtl::Reference<svx::diagram::Point> xAbove(
                    rDgm.getData()->getPointByModelID(aAbove));
                if (!xAbove.is())
                    break;
                nDirection = xAbove->getLayoutVariables().mnDirection;
            }
            return compareResult(maCond.mnOp, nDirection, maCond.mnVal);
        }
        else if (maCond.mnArg == XML_hierBranch)
        {
            sal_Int32 nHierarchyBranch
                    = rPresPoint->getLayoutVariables().moHierarchyBranch.value_or(XML_std);
            if (!rPresPoint->getLayoutVariables().moHierarchyBranch.has_value())
            {
                // If <dgm:hierBranch> is missing in the current presentation
                // point, ask the parent.
                OUString aParent = navigate(rDgm, svx::diagram::TypeConstant::XML_presParOf,
                                            rPresPoint->msModelId,
                                            /*bSourceToDestination*/ false);
                const rtl::Reference<svx::diagram::Point> it(
                    rDgm.getData()->getPointByModelID(aParent));
                if (it.is())
                {
                    if (it->getLayoutVariables().moHierarchyBranch.has_value())
                        nHierarchyBranch = it->getLayoutVariables().moHierarchyBranch.value();
                }
            }
            return compareResult(maCond.mnOp, nHierarchyBranch, maCond.mnVal);
        }
        break;
    }

    case XML_cnt:
        return compareResult(maCond.mnOp, getNodeCount(rDgm, aAsk), maCond.msVal.toInt32());

    case XML_maxDepth:
    {
        // The depth is asked about what the axis of the condition reaches: "axis=root des" asks
        // about the whole tree from its root, and not about the node the pass stands on.
        OUString aFrom(aAsk);
        if (!maIter.maAxis.empty() && maIter.maAxis[0] == XML_root)
        {
            const rtl::Reference<svx::diagram::Point> xRoot(rDgm.getData()->getRootPoint());
            if (xRoot.is())
                aFrom = xRoot->msModelId;
        }
        sal_Int32 nMaxDepth = calcMaxDepth(aFrom, rDgm.getData()->getConnections());
        return compareResult(maCond.mnOp, nMaxDepth, maCond.msVal.toInt32());
    }

    case XML_pos:
    case XML_revPos:
    case XML_posEven:
    case XML_posOdd:
    {
        sal_Int32 nPosition(0);
        sal_Int32 nSiblings(0);
        getNodePlace(rDgm, aAsk, nPosition, nSiblings);
        if (nPosition < 1)
            break;

        // revPos counts from the end, so the last one is the first
        if (maCond.mnFunc == XML_revPos)
            nPosition = nSiblings - nPosition + 1;

        if (maCond.mnFunc == XML_posEven || maCond.mnFunc == XML_posOdd)
        {
            const sal_Int32 nOdd(nPosition % 2);
            return compareResult(maCond.mnOp, maCond.mnFunc == XML_posOdd ? nOdd : 1 - nOdd,
                                 maCond.msVal.toInt32());
        }

        return compareResult(maCond.mnOp, nPosition, maCond.msVal.toInt32());
    }

    case XML_depth:
        // TODO
    default:
        SAL_WARN("oox.drawingml", "unknown function " << maCond.mnFunc);
        break;
    }

    return true;
}

void ConditionAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void ConstraintAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void RuleAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void ConstraintAtom::parseConstraint(std::vector<Constraint>& rConstraints,
                                     bool bRequireForName) const
{
    // Allowlist for cases where empty forName is handled.
    if (bRequireForName)
    {
        switch (maConstraint.mnType)
        {
            case XML_sp:
            case XML_sibSp:
            case XML_lMarg:
            case XML_rMarg:
            case XML_tMarg:
            case XML_bMarg:
            // a snake states its own step aside, "alignOff val=1", for itself and names nothing
            case XML_alignOff:
                bRequireForName = false;
                break;
        }

        // A layout may work a value out once and give it a name of its own, userA and the rest,
        // to state the size of several shapes as parts of it. Such a constraint states the value
        // and names no shape, so it has to come through without one.
        if (isUserVariable(maConstraint.mnType))
            bRequireForName = false;
        // Naming a data point type picks out the children just as exactly as a name does, so
        // such a constraint is passed on as well. Only the catch-all type says nothing.
        if (maConstraint.mnPointType != XML_all)
            bRequireForName = false;
        // A constraint for ch with no name states its value for every child at once, "w for ch
        // refType=w fact=0.19" gives every cell of a snake that width, so it comes through too.
        if (maConstraint.mnFor == XML_ch)
            bRequireForName = false;
    }

    if (bRequireForName && maConstraint.msForName.isEmpty())
        return;

    // accepting only basic equality constraints
    if ((maConstraint.mnOperator == XML_none || maConstraint.mnOperator == XML_equ)
        && maConstraint.mnType != XML_none)
    {
        rConstraints.push_back(maConstraint);
        rConstraints.back().msStatedBy = getLayoutNode().getName();
    }
}

void RuleAtom::parseRule(std::vector<Rule>& rRules) const
{
    if (!maRule.msForName.isEmpty())
    {
        rRules.push_back(maRule);
    }
}

void AlgAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

sal_Int32 AlgAtom::getConnectorType()
{
    sal_Int32 nConnRout = 0;
    sal_Int32 nBegSty = 0;
    sal_Int32 nEndSty = 0;
    if (maMap.count(oox::XML_connRout))
        nConnRout = maMap.find(oox::XML_connRout)->second;
    if (maMap.count(oox::XML_begSty))
        nBegSty = maMap.find(oox::XML_begSty)->second;
    if (maMap.count(oox::XML_endSty))
        nEndSty = maMap.find(oox::XML_endSty)->second;

    if (nConnRout == oox::XML_bend)
        return 0; // was oox::XML_bentConnector3 - connectors are hidden in org chart as they don't work anyway
    // A connector that says it has no head at either end, begSty and endSty noArr, is a plain
    // bar, the rect preset: Vertical_Bending_Process draws its three that way. One that says
    // nothing is an arrow to the right, as before.
    if (nBegSty == oox::XML_noArr && nEndSty == oox::XML_noArr)
        return oox::XML_rect;
    if (nBegSty == oox::XML_arr && nEndSty == oox::XML_arr)
        return oox::XML_leftRightArrow;
    if (nBegSty == oox::XML_arr)
        return oox::XML_leftArrow;
    if (nEndSty == oox::XML_arr)
        return oox::XML_rightArrow;

    return oox::XML_rightArrow;
}

sal_Int32 AlgAtom::getVerticalShapesCount(const ShapePtr& rShape)
{
    // A connector takes no row of its own, and that holds when it carries a text of its own
    // below it as well. It lies between the rows.
    if (rShape->getSubType() == XML_conn)
        return 0;

    if (rShape->getChildren().empty())
        return 1;

    sal_Int32 nDir = XML_fromL;
    if (mnType == XML_hierRoot)
    {
        // The root stands above its branch, or beside it where hierAlign puts it to a side. A
        // root beside its branch adds no row of its own, so the count has to know which.
        nDir = XML_fromT;
        const sal_Int32 nHierAlign(maMap.count(XML_hierAlign) ? maMap.find(XML_hierAlign)->second
                                                              : 0);
        if (nHierAlign == XML_lCtrCh || nHierAlign == XML_lT || nHierAlign == XML_lB)
            nDir = XML_fromL;
        else if (nHierAlign == XML_rCtrCh || nHierAlign == XML_rT || nHierAlign == XML_rB)
            nDir = XML_fromR;
    }
    else if (maMap.count(XML_linDir))
        nDir = maMap.find(XML_linDir)->second;

    const sal_Int32 nSecDir = maMap.count(XML_secLinDir) ? maMap.find(XML_secLinDir)->second : 0;

    sal_Int32 nCount = 0;
    if (nDir == XML_fromT || nDir == XML_fromB)
    {
        for (const ShapePtr& pChild : rShape->getChildren())
            nCount += pChild->getVerticalShapesCount();
    }
    else if ((nDir == XML_fromL || nDir == XML_fromR) && nSecDir == XML_fromT)
    {
        for (const ShapePtr& pChild : rShape->getChildren())
            nCount += pChild->getVerticalShapesCount();
        nCount = (nCount + 1) / 2;
    }
    else
    {
        for (const ShapePtr& pChild : rShape->getChildren())
            nCount = std::max(nCount, pChild->getVerticalShapesCount());
    }

    return nCount;
}

namespace
{
/// first data node the shape rShape shows, or none
rtl::Reference<svx::diagram::Point> firstDataNodeOf(const SmartArtDiagram& rDgm,
                                                    const ShapePtr& rShape)
{
    const PresPointShapeMap& rPresPointShapeMap = rDgm.getLayout()->getPresPointShapeMap();
    const DiagramData_oox::StringMap& rPresOfNameMap = rDgm.getData()->getPresOfNameMap();
    // Get the first presentation node of the shape.
    rtl::Reference<svx::diagram::Point> xPresNode;
    for (const auto& rPair : rPresPointShapeMap)
    {
        if (rPair.second == rShape)
        {
            xPresNode = rPair.first;
            break;
        }
    }
    // Get the first data node of the presentation node.
    rtl::Reference<svx::diagram::Point> xDataNode;
    if (xPresNode.is())
    {
        auto itPresToData = rPresOfNameMap.find(xPresNode->msModelId);
        if (itPresToData != rPresOfNameMap.end())
        {
            for (const auto& rPair : itPresToData->second)
            {
                const DiagramData_oox::SourceIdAndDepth& rItem = rPair.second;
                const rtl::Reference<svx::diagram::Point> it(
                    rDgm.getData()->getPointByModelID(rItem.msSourceId));
                if (it.is())
                {
                    xDataNode = it;
                    break;
                }
            }
        }
    }

    return xDataNode;
}

/// Does the first data node of this shape have customized text properties?
bool HasCustomText(const SmartArtDiagram& rDgm, const ShapePtr& rShape)
{
    const rtl::Reference<svx::diagram::Point> xDataNode(firstDataNodeOf(rDgm, rShape));
    return xDataNode.is() && xDataNode->mbCustomText;
}

// The part of a shape's own size along nRefType that separates it from the next sibling, or
// fDefault when the constraints settle no such part.
double readSpacingFactor(const std::vector<Constraint>& rConstraints, sal_Int32 nRefType,
                         double fDefault)
{
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnType != XML_sp && rConstraint.mnType != XML_sibSp)
            continue;
        if (rConstraint.mnRefType == nRefType && rConstraint.mfFactor > 0.0)
            return rConstraint.mfFactor;
    }
    return fDefault;
}

// A hanging list: a root with its children stacked below it. The layout states the children's
// cell as a part of the root's cell, "h for des forName=childComposite refType=h refFor=des
// refForName=rootComposite fact=0.5205", the gap between the root and its children as sp, and the
// gap between the children as the sibSp for the branch by name, or none where it states none.
// The three come out as parts of the root cell's height, the child gap as a part of the child's.
struct HangingWeights
{
    double fChildOfRoot;
    double fRootGapOfRoot;
    double fChildGapOfChild;
    OUString aRootCell;
};

std::optional<HangingWeights> readHangingWeights(const std::vector<Constraint>& rConstraints,
                                                 std::u16string_view rChildCell,
                                                 std::u16string_view rBranch)
{
    HangingWeights aOut{ 1.0, 0.0, 0.0, OUString() };
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == XML_h && rConstraint.mnFor == XML_des
            && rConstraint.msForName == rChildCell && rConstraint.mnRefType == XML_h
            && !rConstraint.msRefForName.isEmpty() && rConstraint.msRefForName != rChildCell)
        {
            aOut.fChildOfRoot = rConstraint.mfFactor > 0.0 ? rConstraint.mfFactor : 1.0;
            aOut.aRootCell = rConstraint.msRefForName;
            break;
        }
    if (aOut.aRootCell.isEmpty())
        return std::nullopt;

    // a gap stated as a part of the root cell's height, or of the child cell's, as a part of
    // the root cell's
    const auto aOfRoot = [&](const Constraint& rConstraint) -> std::optional<double> {
        if (rConstraint.mnRefType != XML_h || rConstraint.mfFactor <= 0.0)
            return std::nullopt;
        if (rConstraint.msRefForName == aOut.aRootCell)
            return rConstraint.mfFactor;
        if (rConstraint.msRefForName == rChildCell)
            return rConstraint.mfFactor * aOut.fChildOfRoot;
        return std::nullopt;
    };
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == XML_sp && rConstraint.mnFor == XML_des)
            if (const std::optional<double> oGap = aOfRoot(rConstraint))
            {
                aOut.fRootGapOfRoot = *oGap;
                break;
            }
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == XML_sibSp && rConstraint.mnFor == XML_des
            && rConstraint.msForName == rBranch)
            if (const std::optional<double> oGap = aOfRoot(rConstraint))
            {
                aOut.fChildGapOfChild = *oGap / aOut.fChildOfRoot;
                break;
            }
    return aOut;
}

// The part of its own width that a shape takes as its height, or 0 when the constraints tie the
// two together nowhere. A shape that has such a part holds those proportions, and the room it is
// given only sets an upper bound on it.
double readHeightOfOwnWidthFactor(const std::vector<Constraint>& rConstraints)
{
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.msRefForName.isEmpty() && rConstraint.mfFactor > 0.0)
        {
            if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w)
                return rConstraint.mfFactor;

            // A layout may state the proportions the other way about, the width as a part of the
            // height, and a hierarchy commonly does. It says the same thing turned over.
            if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_h)
                return 1.0 / rConstraint.mfFactor;
        }
    }

    return 0.0;
}

/**
 * The constraints a layout node states for itself. A loop or a choose can stand between, so the
 * walk goes on through those, and it stops at a layout node of its own because from there on the
 * constraints belong to that one.
 */
void gatherOwnConstraints(const LayoutAtom& rAtom, std::vector<Constraint>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;

        if (const ConstraintAtom* pConstraint = dynamic_cast<const ConstraintAtom*>(pChild.get()))
            pConstraint->parseConstraint(rOut, /*bRequireForName*/ false);
        else
            gatherOwnConstraints(*pChild, rOut);
    }
}

/**
 * The height each layout node below rAtom states as a part of its own width, by the name of that
 * node. A node that states nothing about its proportions is not in the map.
 */
void gatherHeightOfWidthFactors(const LayoutAtom& rAtom, std::map<OUString, double>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get());
        if (!pNode)
        {
            gatherHeightOfWidthFactors(*pChild, rOut);
            continue;
        }

        std::vector<Constraint> aOwn;
        gatherOwnConstraints(*pNode, aOwn);

        double fFactor(readHeightOfOwnWidthFactor(aOwn));

        // A composite that states nothing for itself may state its children as parts of its
        // width both ways, "w for ch visible refType w" and "h for ch visible refType w": the
        // middle of Radial_List holds a circle as wide as itself, so it is as high as wide. The
        // tallest such child says how high the composite is, and it wins over a smaller part a
        // child of its own states, the hidden circle of 0.7 in the same middle.
        {
            std::vector<const AlgAtom*> aAlgorithms;
            gatherOwnAlgorithms(*pNode, aAlgorithms);
            const bool bComposite(!aAlgorithms.empty()
                                  && std::all_of(aAlgorithms.begin(), aAlgorithms.end(),
                                                 [](const AlgAtom* pAlg) {
                                                     return pAlg->getType() == XML_composite;
                                                 }));
            if (bComposite)
            {
                // A child's width as a part of the composite's, or of another child's width,
                // and its height as a part of the composite's width or of a child's width, its
                // own among them: Radial_List's node holds a parent text 0.4 of the node wide
                // and as high as wide, so the node is 0.4 of its width high, and the ring fits
                // its nodes at that height, not at the plain one. The constraints of both
                // branches of a choose are read, aOwn holds them all.
                std::map<OUString, double> aWidthOfComposite;
                for (int nPass = 0; nPass < 4; ++nPass)
                    for (const Constraint& rConstraint : aOwn)
                    {
                        if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
                            || rConstraint.mnFor != XML_ch || rConstraint.msForName.isEmpty()
                            || rConstraint.mfFactor < 0.0)
                            continue;
                        const double fPart(rConstraint.mfFactor != 0.0 ? rConstraint.mfFactor
                                                                       : 1.0);
                        if (rConstraint.msRefForName.isEmpty())
                            aWidthOfComposite[rConstraint.msForName] = fPart;
                        else if (aWidthOfComposite.count(rConstraint.msRefForName))
                            aWidthOfComposite[rConstraint.msForName]
                                = fPart * aWidthOfComposite.at(rConstraint.msRefForName);
                    }
                for (const Constraint& rConstraint : aOwn)
                {
                    if (rConstraint.mnType != XML_h || rConstraint.mnRefType != XML_w
                        || rConstraint.mnFor != XML_ch || rConstraint.msForName.isEmpty()
                        || rConstraint.mfFactor < 0.0)
                        continue;
                    const double fPart(rConstraint.mfFactor != 0.0 ? rConstraint.mfFactor : 1.0);
                    if (rConstraint.msRefForName.isEmpty())
                        fFactor = std::max(fFactor, fPart);
                    else if (aWidthOfComposite.count(rConstraint.msRefForName))
                        fFactor = std::max(fFactor,
                                           fPart * aWidthOfComposite.at(rConstraint.msRefForName));
                }
            }
        }

        if (fFactor > 0.0)
            rOut[pNode->getName()] = fFactor;
    }
}

/**
 * The height each layout node below rAtom states outright for itself, in EMU, by the name of
 * that node. A height is stated outright with a val and nothing it refers to, and it is written
 * in millimetres. A node that states none is not in the map.
 */
void gatherStatedHeights(const LayoutAtom& rAtom, std::map<OUString, double>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get());
        if (!pNode)
        {
            gatherStatedHeights(*pChild, rOut);
            continue;
        }

        std::vector<Constraint> aOwn;
        gatherOwnConstraints(*pNode, aOwn);
        for (const Constraint& rConstraint : aOwn)
            if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_none
                && rConstraint.msRefForName.isEmpty() && rConstraint.mfValue > 0.0)
                rOut[pNode->getName()]
                    = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm, o3tl::Length::emu);
    }
}

/**
 * The sites a connector below rAtom starts at and ends at, begPts and endPts of its algorithm,
 * by the name of that layout node. A connector that states neither is not in the map, one that
 * states only one has 0 for the other.
 */
void gatherEndSites(const LayoutAtom& rAtom,
                    std::map<OUString, std::pair<sal_Int32, sal_Int32>>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get());
        if (!pNode)
        {
            gatherEndSites(*pChild, rOut);
            continue;
        }

        for (const LayoutAtomPtr& pBelow : pNode->getChildren())
        {
            const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pBelow.get());
            if (!pAlg)
                continue;
            const AlgAtom::ParamMap& rMap(pAlg->getMap());
            const auto aBegin = rMap.find(XML_begPts);
            const auto aEnd = rMap.find(XML_endPts);
            if (aBegin != rMap.end() || aEnd != rMap.end())
                rOut[pNode->getName()] = std::make_pair(aBegin != rMap.end() ? aBegin->second : 0,
                                                        aEnd != rMap.end() ? aEnd->second : 0);
        }
    }
}

/**
 * The part of the way a connector below rAtom leaves free at its start, begPad stated as a part
 * of connDist, by the name of that layout node. A connector that states none is not in the map.
 */
void gatherBeginPads(const LayoutAtom& rAtom, std::map<OUString, double>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get());
        if (!pNode)
        {
            gatherBeginPads(*pChild, rOut);
            continue;
        }

        std::vector<Constraint> aOwn;
        gatherOwnConstraints(*pNode, aOwn);
        for (const Constraint& rConstraint : aOwn)
            if (rConstraint.mnType == XML_begPad && rConstraint.mnRefType == XML_connDist
                && rConstraint.mfFactor > 0.0)
                rOut[pNode->getName()] = rConstraint.mfFactor;
    }
}

// The height the shape named rName states as a part of its own width with its name on both
// sides, w for des level1Shape refType=h refFor=des refForName=level1Shape fact=2, which a
// hierarchy does for a shape levels below it. 0 where it states none that way.
// True for a constraint about the shape named rName: by its name, or for every node, for des
// ptType=node with no name, where the shape stands for a node.
bool isConstraintFor(const Constraint& rConstraint, std::u16string_view rName)
{
    return rConstraint.msForName == rName
           || (rConstraint.msForName.isEmpty() && rConstraint.mnPointType == XML_node);
}

// True for a constraint whose reference is the shape named rName itself, by name or as every
// node, refPtType=node with no name.
bool constraintRefersToItself(const Constraint& rConstraint, std::u16string_view rName)
{
    return rConstraint.msRefForName == rName
           || (rConstraint.msRefForName.isEmpty() && rConstraint.mnRefPointType == XML_node);
}

double readHeightOfWidthFactorOf(const std::vector<Constraint>& rConstraints,
                                 std::u16string_view rName)
{
    for (const Constraint& rConstraint : rConstraints)
    {
        if (!isConstraintFor(rConstraint, rName) || !constraintRefersToItself(rConstraint, rName)
            || rConstraint.mfFactor <= 0.0)
            continue;
        if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w)
            return rConstraint.mfFactor;
        if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_h)
            return 1.0 / rConstraint.mfFactor;
    }
    return 0.0;
}

// What the constraints state for the shape named rName as nWhat, w or h: a part of another
// shape's w or h by name, or of the room's where no shape is named. The other shape's sizes are
// looked up in rKnown, by name. False where nothing is stated or the other shape is not known.
bool readStatedSize(const std::vector<Constraint>& rConstraints, std::u16string_view rName,
                    sal_Int32 nWhat, const std::map<OUString, awt::Size>& rKnown,
                    const awt::Size& rRoom, double& rOut)
{
    for (const Constraint& rConstraint : rConstraints)
    {
        if (!isConstraintFor(rConstraint, rName) || rConstraint.mnType != nWhat
            || (rConstraint.mnRefType != XML_w && rConstraint.mnRefType != XML_h)
            || constraintRefersToItself(rConstraint, rName))
            continue;
        const double fFactor(rConstraint.mfFactor > 0.0 ? rConstraint.mfFactor : 1.0);
        if (rConstraint.msRefForName.isEmpty())
        {
            rOut = fFactor * (rConstraint.mnRefType == XML_w ? rRoom.Width : rRoom.Height);
            return true;
        }
        const auto aOther = rKnown.find(rConstraint.msRefForName);
        if (aOther == rKnown.end())
            continue;
        rOut = fFactor
               * (rConstraint.mnRefType == XML_w ? aOther->second.Width : aOther->second.Height);
        return true;
    }
    return false;
}

// The part of the width of a hierarchy branch that the child shapes take, or 0 when the
// constraints settle no such part. The width left over is the lane that carries the connectors.
// Only a constraint about one of the branch's own children counts; the constraints of the whole
// hierarchy ride along and one of them may state a width for some shape elsewhere.
double readChildOfBranchWidthFactor(const std::vector<Constraint>& rConstraints,
                                    const ShapePtr& rBranch)
{
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
            || rConstraint.msRefForName.isEmpty() || rConstraint.mfFactor <= 0.0
            || rConstraint.mfFactor >= 1.0)
            continue;
        bool bAboutAChild(rConstraint.msForName.isEmpty());
        for (const ShapePtr& rChild : rBranch->getChildren())
            bAboutAChild = bAboutAChild || rChild->getInternalName() == rConstraint.msForName;
        if (bAboutAChild)
            return rConstraint.mfFactor;
    }
    return 0.0;
}

// The constraints that rLayoutNode states for itself, the ones that name no shape included.
std::vector<Constraint> collectDirectConstraints(const LayoutNode& rLayoutNode)
{
    std::vector<Constraint> aConstraints;
    for (const LayoutAtomPtr& pChild : rLayoutNode.getChildren())
    {
        auto pConstraintAtom = dynamic_cast<ConstraintAtom*>(pChild.get());
        if (pConstraintAtom)
            pConstraintAtom->parseConstraint(aConstraints, /*bRequireForName=*/false);
    }
    return aConstraints;
}

// The constraints that each layout node one step below rAtom states for itself, under the name of
// that layout node. A for-each or a choose on the way down counts as no step, and a layout node
// below the first one is left out.
void collectChildConstraints(const LayoutAtom& rAtom,
                             std::map<OUString, std::vector<Constraint>>& rResult)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (auto pLayoutNode = dynamic_cast<LayoutNode*>(pChild.get()))
        {
            rResult[pLayoutNode->getName()] = collectDirectConstraints(*pLayoutNode);
            continue;
        }

        if (dynamic_cast<ForEachAtom*>(pChild.get()) || dynamic_cast<ChooseAtom*>(pChild.get())
            || dynamic_cast<ConditionAtom*>(pChild.get()))
            collectChildConstraints(*pChild, rResult);
    }
}

/// True when the layout node rNode states a rule of type nType that lets the value grow without
/// bound, "rule type=w val=INF". The rules are read from the node's own atoms, in every branch of
/// a choose, and not from the layout nodes below it.
bool ownRuleGrows(const LayoutAtom& rAtom, sal_Int32 nType)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const RuleAtom* pRule = dynamic_cast<const RuleAtom*>(pChild.get()))
        {
            const Rule& rRule = pRule->getRule();
            if (rRule.mnType == nType && std::isinf(rRule.mfValue) && rRule.mfValue > 0.0)
                return true;
        }
        else if (ownRuleGrows(*pChild, nType))
            return true;
    }
    return false;
}

/// True when the layout node rNode states a rule of type nType with a finite value, a step the
/// value may grow to before the next rule, "rule type=h val=100" on Vertical_Chevron_List's
/// chevrons. Read like ownRuleGrows.
bool ownRuleStatesFinite(const LayoutAtom& rAtom, sal_Int32 nType)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const RuleAtom* pRule = dynamic_cast<const RuleAtom*>(pChild.get()))
        {
            const Rule& rRule = pRule->getRule();
            if (rRule.mnType == nType && std::isfinite(rRule.mfValue))
                return true;
        }
        else if (ownRuleStatesFinite(*pChild, nType))
            return true;
    }
    return false;
}

// True when the layout node states a rule of type nType that frees the value to follow the
// content: "val=INF", or a factor with no value, "rule type=w fact=1" on Accented_Picture's
// child texts, which the drawing has as wide as their text.
bool ownRuleFrees(const LayoutAtom& rAtom, sal_Int32 nType)
{
    if (ownRuleGrows(rAtom, nType))
        return true;
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const RuleAtom* pRule = dynamic_cast<const RuleAtom*>(pChild.get()))
        {
            const Rule& rRule = pRule->getRule();
            if (rRule.mnType == nType && !std::isfinite(rRule.mfValue)
                && std::isfinite(rRule.mfFactor) && rRule.mfFactor > 0.0)
                return true;
        }
        else if (ownRuleFrees(*pChild, nType))
            return true;
    }
    return false;
}

// the factor a rule of type nType lets the value grow by (a rule with a factor and no
// value), and the font the rules before it let the text shrink to first, the last "rule
// type=primFontSz" before it; nothing where the node states none
std::optional<std::pair<double, double>> ownRuleFactorOf(const LayoutAtom& rAtom, sal_Int32 nType)
{
    double fFloor(0.0);
    std::optional<std::pair<double, double>> oFound;
    std::function<void(const LayoutAtom&)> aWalk = [&](const LayoutAtom& rBelow) {
        for (const LayoutAtomPtr& pChild : rBelow.getChildren())
        {
            if (oFound)
                return;
            if (dynamic_cast<const LayoutNode*>(pChild.get()))
                continue;
            if (const RuleAtom* pRule = dynamic_cast<const RuleAtom*>(pChild.get()))
            {
                const Rule& rRule = pRule->getRule();
                if (rRule.mnType == XML_primFontSz && std::isfinite(rRule.mfValue))
                    fFloor = rRule.mfValue;
                else if (rRule.mnType == nType && !std::isfinite(rRule.mfValue)
                         && !std::isinf(rRule.mfValue) && std::isfinite(rRule.mfFactor)
                         && rRule.mfFactor > 0.0)
                    oFound = std::make_pair(rRule.mfFactor, fFloor);
            }
            else
                aWalk(*pChild);
        }
    };
    aWalk(rAtom);
    return oFound;
}

/// The extent each child of a linear layout asks for along the axis the children are placed on,
/// as a multiple of the parent extent. A child states its wish as a factor of a reference: the
/// parent, or an other child named directly or addressed by its data point type. A reference may
/// point at a child whose own wish is not worked out yet, so the wishes are collected in several
/// passes, until a pass adds nothing new.
class LinearChildExtents
{
public:
    /// nAxisType is XML_w or XML_h, nParentExtent the parent extent along that axis, in EMU,
    /// and nOtherExtent the parent extent across it.
    void read(const SmartArtDiagram& rDgm, const LayoutNode& rRowNode, const ShapePtr& rRow,
              const std::vector<Constraint>& rConstraints, sal_Int32 nAxisType,
              sal_Int32 nParentExtent, sal_Int32 nOtherExtent);

    /// True when at least one constraint stated an extent.
    bool isFilled() const { return mbFilled; }

    /// True when a wish walks back at least half of the largest wish, so that what follows lies
    /// over a child rather than a little closer to it.
    bool hasOverlap() const { return mbOverlap; }

    /// True when a child's wish along the axis, as it finally stands, was stated as a part of
    /// the parent's own extent, a share, and not as a length against some other shape or a name
    /// of the layout's own. A share that a later constraint or a set overrides is none.
    bool hasShareOfParent() const { return !maShareNames.empty() || mbShareByType; }

    /// True when a wish points backwards, which means the children are meant to overlap.
    bool hasBackwards() const { return mbBackwards; }

    /// True when the wish of the child named rName along the axis was stated as a bare length
    /// in millimetres, which keeps its length where the row scales the parts to fit.
    bool isFixedLength(const OUString& rName) const { return maFixedNames.count(rName) > 0; }

    /// The extent rShape asks for, as a multiple of the parent extent, or nothing when no
    /// constraint mentions that child.
    std::optional<double> get(const oox::drawingml::Shape& rShape) const;

    /// True when the wish of the child named rName along the axis was stated as a part of that
    /// child's own extent across, a composite 0.4986 of its own height wide.
    bool isTiedToOwnCross(const OUString& rName) const { return maTiedToOwnCross.count(rName) > 0; }

    /// The extent stated for rName along the axis, a child's or a node's below, as a part of the
    /// parent's extent, or nothing.
    std::optional<double> stated(const OUString& rName) const
    {
        const auto aOwn = maByName.find(rName);
        if (aOwn != maByName.end())
            return aOwn->second;
        const auto aBelow = maBelowByName.find(rName);
        if (aBelow != maBelowByName.end())
            return aBelow->second;
        return std::nullopt;
    }

    /// The extent stated for rName across the axis, as a part of the parent's extent across,
    /// or nothing.
    std::optional<double> statedAcross(const OUString& rName) const
    {
        const auto aOwn = maOtherByName.find(rName);
        if (aOwn != maOtherByName.end())
            return aOwn->second;
        return std::nullopt;
    }

    /// The wishes left out because they are parts of a shape a hierarchy in the row lays out,
    /// by the child's name: the constraint as stated.
    const std::map<OUString, Constraint>& fitDeferred() const { return maFitDeferred; }

    /// States the extent of the child named rName outright, as a part of the parent's extent.
    /// With bKeepShare the child stays a share of the parent where it was one: the extent is
    /// only how far its content reaches within the share it asked for.
    void set(const OUString& rName, double fFactor, bool bKeepShare)
    {
        maByName[rName] = fFactor;
        if (!bKeepShare)
            maShareNames.erase(rName);
        mbFilled = true;
    }

    /// Reads the overlap and the backwards walk off the wishes as they stand now, for the
    /// children rChildren: a wish of less than nothing is a walk back, and one that walks back
    /// at least half of the largest wish is an overlap. The wishes a row sets for its children
    /// after the reading may be smaller than the bounds the reading saw.
    void refreshOverlap(const std::vector<ShapePtr>& rChildren)
    {
        double fLargest(0.0);
        double fMostBack(0.0);
        for (const ShapePtr& pChild : rChildren)
        {
            const std::optional<double> oWish(get(*pChild));
            if (!oWish)
                continue;
            fLargest = std::max(fLargest, *oWish);
            fMostBack = std::min(fMostBack, *oWish);
        }
        mbBackwards = fMostBack < 0.0;
        mbOverlap = fMostBack < 0.0 && -fMostBack >= fLargest / 2.0;
    }

    /// States the extent of the one child rShape outright, as a part of the parent's extent,
    /// where the children of one name differ: a row of columns named alike, each as wide as the
    /// row below it needs.
    void setForShape(const oox::drawingml::Shape& rShape, double fFactor)
    {
        maByShape[&rShape] = fFactor;
        maShareNames.erase(rShape.getInternalName());
        mbFilled = true;
    }

private:
    std::map<OUString, double> maByName;
    std::map<sal_Int32, double> maByPointType;
    /// the extents stated for one child each, where children of one name differ
    std::map<const oox::drawingml::Shape*, double> maByShape;
    /// The extents stated from here for the nodes below the row's children, for des, as parts of
    /// the row's extent. No child takes one; a child's extent that refers to such a name reads it.
    std::map<OUString, double> maBelowByName;
    /// The same three, read across the axis, as parts of the parent's extent across. A wish
    /// along the axis may be stated as a part of one of these, a width as a part of a height.
    std::map<OUString, double> maOtherByName;
    std::map<sal_Int32, double> maOtherByPointType;
    std::map<OUString, double> maOtherBelowByName;
    bool mbFilled = false;
    bool mbBackwards = false;
    bool mbOverlap = false;
    std::set<OUString> maShareNames;
    /// the names whose wish along the axis is a bare length in millimetres
    std::set<OUString> maFixedNames;
    bool mbShareByType = false;
    std::map<OUString, Constraint> maFitDeferred;
    std::set<OUString> maTiedToOwnCross;
};

void LinearChildExtents::read(const SmartArtDiagram& rDgm, const LayoutNode& rRowNode,
                              const ShapePtr& rRow, const std::vector<Constraint>& rConstraints,
                              sal_Int32 nAxisType, sal_Int32 nParentExtent,
                              sal_Int32 nOtherExtent)
{
    if (nParentExtent <= 0)
        return;

    // The nodes below a child that lays out a hierarchy get their size from the fit of that
    // hierarchy into the child, not from what the constraints state for them: Labeled_Hierarchy
    // states level1Shape as wide as the whole and 0.66667 of that high, and the fit makes it a
    // fifth of that. A wish stated as a part of such a node is unknown here, so it is left out,
    // a spacer of 0.1 of the node above the hierarchy among them.
    std::set<OUString> aFitDecided;
    for (const LayoutAtomPtr& pChild : rRowNode.getChildren())
    {
        const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get());
        if (!pNode || !hasHierarchyBelow(*pNode))
            continue;
        std::map<OUString, const LayoutNode*> aBelow;
        gatherLayoutNodes(*pNode, aBelow);
        for (const auto& rEntry : aBelow)
            aFitDecided.insert(rEntry.first);
    }
    const auto aIsChild = [&rRow](std::u16string_view rName) {
        for (const ShapePtr& pChild : rRow->getChildren())
            if (pChild->getInternalName() == rName)
                return true;
        return false;
    };
    const sal_Int32 nOffsetType(nAxisType == XML_h ? XML_hOff : XML_wOff);
    const sal_Int32 nOtherType(nAxisType == XML_h ? XML_w : XML_h);

    // The wishes along the axis are what the row hands out. The wishes across it are read as
    // well, each as a part of the parent's extent across, for a wish along the axis may be
    // stated as a part of one across: a composite as wide as 0.4986 of its own height, the
    // height being the whole of the row's.
    struct Maps
    {
        std::map<OUString, double>& rByName;
        std::map<sal_Int32, double>& rByPointType;
        std::map<OUString, double>& rBelowByName;
        sal_Int32 nExtent;
    };
    Maps aAlong{ maByName, maByPointType, maBelowByName, nParentExtent };
    Maps aAcross{ maOtherByName, maOtherByPointType, maOtherBelowByName, nOtherExtent };
    const auto aLookupIn = [](const Maps& rMaps, const OUString& rName, sal_Int32 nPointType,
                              double& rFactor) {
        if (!rName.isEmpty())
        {
            const auto aNamed = rMaps.rByName.find(rName);
            if (aNamed != rMaps.rByName.end())
            {
                rFactor = aNamed->second;
                return true;
            }
            const auto aBelow = rMaps.rBelowByName.find(rName);
            if (aBelow == rMaps.rBelowByName.end())
                return false;
            rFactor = aBelow->second;
            return true;
        }
        const auto aTyped = rMaps.rByPointType.find(nPointType);
        if (aTyped == rMaps.rByPointType.end())
            return false;
        rFactor = aTyped->second;
        return true;
    };

    size_t nKnown = 0;
    for (size_t nPass = 0; nPass < 8; ++nPass)
    {
        for (const Constraint& rConstraint : rConstraints)
        {
            const bool bAlong(rConstraint.mnType == nAxisType);
            if (!bAlong && (rConstraint.mnType != nOtherType || nOtherExtent <= 0))
                continue;
            Maps& rInto(bAlong ? aAlong : aAcross);

            // A negative extent walks the place for the next child back over the one before.
            // That counts however the extent is worked out, so it is looked for on every
            // constraint of the axis, not only on the ones that resolve here.
            if (bAlong && rConstraint.mfFactor < 0.0)
                mbBackwards = true;

            // A constraint for the children of this row, or one stated levels above for every
            // node below by name, for des, where the name is a child of this row. One for des
            // that names no child names a node further below: no child takes it, but a child
            // whose extent refers to that name reads it, the width of a spacer stated as the
            // width of the connector inside it say.
            if (rConstraint.mnFor != XML_ch && rConstraint.mnFor != XML_des)
                continue;
            // An empty name and the catch-all point type together address no single child.
            const bool bByName = !rConstraint.msForName.isEmpty();
            // One for des that names no shape but a point type is for the children of that type
            // as much as for any below: Alternating_Flow's "w for des ptType sibTrans refFor ch
            // refForName composite1 fact 0.05" gives its connectors, children of the row, their
            // slot.
            const bool bBelow(rConstraint.mnFor == XML_des && bByName
                              && !aIsChild(rConstraint.msForName));
            if (!bByName && rConstraint.mnPointType == XML_all)
                continue;

            // A constraint for ch that names a shape which is no child of this row is a node
            // above's for its own children, riding along with the constraints handed down. It is
            // no wish of this row, and it is no reference for one either: the column of a
            // spacer and a connector would otherwise read the width of the node above it as
            // its own width, and every wish stated against that node would come out as small
            // as the column.
            if (rConstraint.mnFor == XML_ch && bByName && !aIsChild(rConstraint.msForName))
                continue;

            if (!bBelow && aFitDecided.count(rConstraint.msRefForName))
            {
                // Labeled_Hierarchy's firstBuf of 0.1 of level1Shape: kept for later, when the
                // hierarchy has laid the shape out
                if (bAlong && bByName && aIsChild(rConstraint.msForName)
                    && rConstraint.mfFactor > 0.0
                    && (rConstraint.mnRefType == XML_w || rConstraint.mnRefType == XML_h))
                    maFitDeferred[rConstraint.msForName] = rConstraint;
                continue;
            }

            double fFactor = 0.0;
            bool bShare(false);
            if (rConstraint.mnRefType == nAxisType || rConstraint.mnRefType == nOtherType)
            {
                // What it refers to is a part of the parent's extent of the referred type, the
                // whole of it where it names nothing; as a part of this type's extent it is
                // scaled by the two extents.
                const Maps& rFrom(rConstraint.mnRefType == nAxisType ? aAlong : aAcross);
                if (rFrom.nExtent <= 0)
                    continue;
                double fReference = 1.0;
                bShare = bAlong && !bBelow && rConstraint.mnRefFor != XML_ch
                         && rConstraint.msRefForName.isEmpty();
                // A wish the other children refer to is a size and no share: Chevron_List's
                // bigChev is the row's whole width, its nodes 0.83 of it and its transitions
                // -0.13 and -0.14 of those, 3.13 widths in all, and the drawing shrinks them
                // all alike to 0.32, where a share would have taken what the others leave.
                if (bShare && bByName)
                    for (const Constraint& rOther : rConstraints)
                        if (rOther.msRefForName == rConstraint.msForName
                            && rOther.msForName != rConstraint.msForName
                            && (rOther.mnRefType == nAxisType || rOther.mnRefType == nOtherType))
                        {
                            bShare = false;
                            break;
                        }
                // A constraint stated levels above that refers to no shape by name refers to the
                // extent of the node it is stated in, not to this row's: Vertical_Equation's
                // root says its nodes are 0.5 of w high, the root's w, and the column that lays
                // them out is far narrower than that. That node is laid out before the row.
                if (rConstraint.mnRefFor != XML_ch && rConstraint.msRefForName.isEmpty()
                    && !rConstraint.msStatedBy.isEmpty()
                    && rConstraint.msStatedBy != rRow->getInternalName())
                {
                    const std::optional<sal_Int32> aOf(
                        sizeOfLaidOutShape(rDgm, rConstraint.msStatedBy, rConstraint.mnRefType));
                    if (aOf && *aOf > 0)
                    {
                        fReference = static_cast<double>(*aOf) / rFrom.nExtent;
                        // a part of the node above's extent of the same type is a share still,
                        // one of the other type is a length
                        if (rConstraint.mnRefType != nAxisType)
                            bShare = false;
                    }
                }
                if (rConstraint.mnRefFor == XML_ch || !rConstraint.msRefForName.isEmpty())
                {
                    if (!aLookupIn(rFrom, rConstraint.msRefForName, rConstraint.mnRefPointType,
                                   fReference))
                    {
                        // A shape that is no child of this row, the picture beside it say, is
                        // laid out already where the node above placed it before the row, so
                        // its size can be read off it, as a part of the row's extent.
                        const std::optional<sal_Int32> aOf(
                            rConstraint.msRefForName.isEmpty()
                                ? std::nullopt
                                : sizeOfLaidOutShape(rDgm, rConstraint.msRefForName,
                                                     rConstraint.mnRefType));
                        if (!aOf)
                            continue;
                        fReference = static_cast<double>(*aOf) / rFrom.nExtent;
                    }
                }
                fFactor = fReference * rConstraint.mfFactor * rFrom.nExtent / rInto.nExtent;
            }
            else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue != 0.0
                     && std::isfinite(rConstraint.mfValue))
            {
                // A bare value is stated in mm, while a shape extent is always in EMU. A layout
                // may also ask for an endless extent, which states no size at all.
                fFactor = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm, o3tl::Length::emu)
                          / static_cast<double>(rInto.nExtent);
                if (bAlong && !bBelow && bByName)
                    maFixedNames.insert(rConstraint.msForName);
            }
            else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue == 0.0
                     && rConstraint.mnOperator == XML_none)
            {
                // A constraint that refers to nothing and states no value is an extent of
                // nothing: "h for des forName=thickLine" makes the line a line, no height at all,
                // and the row gives the child no room along the axis. One with "op=equ" and
                // nothing else says the children are the same size, not that they have none.
                if (bBelow)
                    rInto.rBelowByName[rConstraint.msForName] = 0.0;
                else if (bByName)
                    rInto.rByName[rConstraint.msForName] = 0.0;
                else
                    rInto.rByPointType[rConstraint.mnPointType] = 0.0;
                continue;
            }
            else if (isUserVariable(rConstraint.mnRefType))
            {
                // A name of the layout's own, userA and the rest, holds a length in EMU.
                const std::optional<sal_Int32> aValue(
                    resolveUserVariable(rDgm, rConstraint.mnRefType, rConstraints));
                if (!aValue)
                    continue;
                fFactor = *aValue * (rConstraint.mfFactor != 0.0 ? rConstraint.mfFactor : 1.0)
                          / static_cast<double>(rInto.nExtent);
            }

            // A wish of no size at all says nothing about the child.
            if (fFactor == 0.0 || !std::isfinite(fFactor))
                continue;

            if (bBelow)
                rInto.rBelowByName[rConstraint.msForName] = fFactor;
            else if (bByName)
                rInto.rByName[rConstraint.msForName] = fFactor;
            else
                rInto.rByPointType[rConstraint.mnPointType] = fFactor;

            // whether the wish, as it stands now, is a share of the parent
            if (bAlong && !bBelow)
            {
                if (bByName)
                {
                    if (rConstraint.mnRefType == nOtherType
                        && rConstraint.msRefForName == rConstraint.msForName)
                        maTiedToOwnCross.insert(rConstraint.msForName);
                    else
                        maTiedToOwnCross.erase(rConstraint.msForName);
                }
                if (bByName)
                {
                    if (bShare)
                        maShareNames.insert(rConstraint.msForName);
                    else
                        maShareNames.erase(rConstraint.msForName);
                }
                else
                    mbShareByType = bShare;
            }
        }

        const size_t nNow = maByName.size() + maByPointType.size() + maBelowByName.size()
                            + maOtherByName.size() + maOtherByPointType.size()
                            + maOtherBelowByName.size();
        if (nNow == nKnown)
            break;
        nKnown = nNow;
    }

    // An offset, hOff beside h or wOff beside w, is added to the extent of the child it names,
    // stated as a part of a name of the layout's own or of the parent's extent.
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnType != nOffsetType || rConstraint.mnFor != XML_ch
            || rConstraint.msForName.isEmpty() || rConstraint.mfFactor == 0.0)
            continue;
        const auto aBase = maByName.find(rConstraint.msForName);
        if (aBase == maByName.end())
            continue;
        double fOffset(0.0);
        if (isUserVariable(rConstraint.mnRefType))
        {
            const std::optional<sal_Int32> aValue(
                resolveUserVariable(rDgm, rConstraint.mnRefType, rConstraints));
            if (!aValue)
                continue;
            fOffset = *aValue * rConstraint.mfFactor / static_cast<double>(nParentExtent);
        }
        else if (rConstraint.mnRefType == nAxisType && rConstraint.msRefForName.isEmpty())
            fOffset = rConstraint.mfFactor;
        else
            continue;
        aBase->second += fOffset;
        if (aBase->second < 0.0)
            mbBackwards = true;
    }

    mbFilled = !maByName.empty() || !maByPointType.empty();

    // A spacer of a little less than nothing pulls the next child a little closer, a gap of
    // -0.035 of a node; one of a whole child less lays what follows over that child. The first
    // is a division of the parent like any other, the second is not.
    double fLargest(0.0);
    for (const auto& rEntry : maByName)
        fLargest = std::max(fLargest, rEntry.second);
    for (const auto& rEntry : maByPointType)
        fLargest = std::max(fLargest, rEntry.second);
    for (const auto& rEntry : maByName)
        if (rEntry.second <= -fLargest / 2.0)
            mbOverlap = true;
    for (const auto& rEntry : maByPointType)
        if (rEntry.second <= -fLargest / 2.0)
            mbOverlap = true;

    // A wish of less than nothing that could not be worked out is not known to be small, so it
    // counts as an overlap as well.
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == nAxisType && rConstraint.mfFactor < 0.0
            && !rConstraint.msForName.isEmpty() && aIsChild(rConstraint.msForName)
            && !maByName.count(rConstraint.msForName)
            && !maBelowByName.count(rConstraint.msForName))
            mbOverlap = true;
}

std::optional<double> LinearChildExtents::get(const oox::drawingml::Shape& rShape) const
{
    // One child stated on its own wins over its name, and a name is the more exact way to
    // address a child, so it wins over the data point type.
    auto aByShape = maByShape.find(&rShape);
    if (aByShape != maByShape.end())
        return aByShape->second;
    auto aByName = maByName.find(rShape.getInternalName());
    if (aByName != maByName.end())
        return aByName->second;

    auto aByPointType = maByPointType.find(rShape.getDataNodeType());
    if (aByPointType != maByPointType.end())
        return aByPointType->second;

    return std::nullopt;
}

// The factor in a layout node's own statement that its size across the axis is a part of its
// size along the axis, "my height is 0.6 of my width" and the other way round. Nothing when the
// node makes no such statement. Only a plain statement counts, one the node makes about itself
// with no operator turning it into a bound.
std::optional<double> readOwnCrossFactor(const std::vector<Constraint>& rChildConstraints,
                                         sal_Int32 nType, sal_Int32 nRefType)
{
    for (const Constraint& rConstraint : rChildConstraints)
    {
        if (rConstraint.mnType != nType || rConstraint.mnRefType != nRefType)
            continue;
        if (rConstraint.mnFor != XML_self || rConstraint.mnRefFor != XML_self)
            continue;
        if (!rConstraint.msForName.isEmpty() || !rConstraint.msRefForName.isEmpty())
            continue;
        if (rConstraint.mnOperator != XML_none || rConstraint.mfFactor <= 0.0)
            continue;

        return rConstraint.mfFactor;
    }

    return std::nullopt;
}

// The size along nType, either XML_w or XML_h, that rChildConstraints ask for inside a shape of
// size rParentSize, or nothing when they ask for no such size. A reference that the shape around
// them hands out, userA for one, takes the value that rParentConstraints give it.
std::optional<sal_Int32> readRequestedSize(const std::vector<Constraint>& rChildConstraints,
                                           const std::vector<Constraint>& rParentConstraints,
                                           sal_Int32 nType, const awt::Size& rParentSize)
{
    for (const Constraint& rConstraint : rChildConstraints)
    {
        if (rConstraint.mnType != nType || rConstraint.mnRefType == XML_none)
            continue;

        double fBase = 0.0;
        if (rConstraint.mnRefType == XML_w)
            fBase = rParentSize.Width;
        else if (rConstraint.mnRefType == XML_h)
            fBase = rParentSize.Height;
        else
        {
            for (const Constraint& rParentConstraint : rParentConstraints)
            {
                if (rParentConstraint.mnType != rConstraint.mnRefType)
                    continue;
                if (rParentConstraint.mnRefType == XML_w)
                    fBase = rParentSize.Width * rParentConstraint.mfFactor;
                else if (rParentConstraint.mnRefType == XML_h)
                    fBase = rParentSize.Height * rParentConstraint.mfFactor;
                break;
            }
        }

        if (fBase <= 0.0)
            continue;

        return static_cast<sal_Int32>(fBase * rConstraint.mfFactor);
    }

    return std::nullopt;
}
}

// True when the layout node draws its shape with the connector algorithm, whichever branch of a
// choose below it that stands in. The walk stops at a layout node of its own.
// Whether the connector a layout node draws is a line, "dim val=1D", read through a choose.
static bool drawsALine(const LayoutAtom& rAtom)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
        {
            const auto aDim = pAlg->getMap().find(XML_dim);
            if (pAlg->getType() == XML_conn && aDim != pAlg->getMap().end()
                && aDim->second == XML_1D)
                return true;
            continue;
        }
        if (drawsALine(*pChild))
            return true;
    }
    return false;
}

static bool drawsAConnector(const LayoutAtom& rAtom)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
        {
            if (pAlg->getType() == XML_conn)
                return true;
            continue;
        }
        if (drawsAConnector(*pChild))
            return true;
    }
    return false;
}

// The route of the connector algorithm below the layout node, connRout, or stra where it states
// none, and 0 where no connector algorithm stands below it. The walk stops at a layout node.
static sal_Int32 connectorRouteOf(const LayoutAtom& rAtom)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
        {
            if (pAlg->getType() != XML_conn)
                continue;
            const auto aRoute = pAlg->getMap().find(XML_connRout);
            return aRoute == pAlg->getMap().end() ? XML_stra : aRoute->second;
        }
        const sal_Int32 nBelow(connectorRouteOf(*pChild));
        if (nBelow != 0)
            return nBelow;
    }
    return 0;
}

// The nodes are the cells of the grid, and a transition stands in the gap between the two nodes it
// joins. Along a line that gap is as wide as the transition states for itself, a part of the node,
// and between two lines it is sp. Every node is one node width wide and as high as the tallest of
// them states as a part of its width. Of the grids that hold the nodes the one that leaves them
// largest is taken, or as many per line as bkpt says, and the whole stands in the middle of the
// room. A line runs the way of the flow, the next one the same way or back, as contDir says.
bool SnakeAlg::layoutBendingProcess(const AlgAtom& rAlg, const ShapePtr& rShape,
                                    const std::vector<Constraint>& rConstraints)
{
    std::vector<ShapePtr> aNodes;
    std::vector<ShapePtr> aTransitions;
    // A transition is what stands for a sibTrans, and this is the snake of a process only when
    // at least one of them is drawn by the connector algorithm. The shape carries the type it is
    // drawn as, a triangle say, so the algorithm is asked of the layout node. A snake whose
    // transitions are spaces or nothing at all, the lists, is not this kind.
    std::map<OUString, const LayoutNode*> aLayoutNodes;
    gatherLayoutNodes(rAlg.getLayoutNode(), aLayoutNodes);
    bool bConnectors(false);
    for (const ShapePtr& rChild : rShape->getChildren())
    {
        if (rChild->getDataNodeType() == XML_sibTrans && rChild->getSubType() != XML_sp)
        {
            aTransitions.push_back(rChild);
            const auto aNode = aLayoutNodes.find(rChild->getInternalName());
            bConnectors = bConnectors
                          || (aNode != aLayoutNodes.end() && aNode->second
                              && drawsAConnector(*aNode->second));
        }
        else
            aNodes.push_back(rChild);
    }
    if (!bConnectors || aNodes.empty())
        return false;

    const AlgAtom::ParamMap& rMap(rAlg.getMap());
    const auto aParam = [&rMap](sal_Int32 nWhat, sal_Int32 nElse) {
        const auto aFound = rMap.find(nWhat);
        return aFound == rMap.end() ? nElse : aFound->second;
    };
    const bool bRows(aParam(XML_flowDir, XML_row) != XML_col);
    const bool bBack(aParam(XML_contDir, XML_sameDir) == XML_revDir);
    const sal_Int32 nGrowDir(aParam(XML_grDir, XML_tL));
    const sal_Int32 nBreak(aParam(XML_bkpt, XML_endCnv));
    const sal_Int32 nBreakAt(aParam(XML_bkPtFixedVal, 2));
    const sal_Int32 nCount(aNodes.size());

    const auto aIsNode = [&aNodes](const OUString& rName) {
        for (const ShapePtr& rNode : aNodes)
            if (rNode->getInternalName() == rName)
                return true;
        return false;
    };

    // The height of a node as a part of its width, stated by the node itself or by the snake
    // for it. The tallest sets the cell.
    std::map<OUString, double> aHeightOfWidth;
    gatherHeightOfWidthFactors(rAlg.getLayoutNode(), aHeightOfWidth);
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w
            && !rConstraint.msForName.isEmpty() && rConstraint.msRefForName.isEmpty()
            && rConstraint.mfFactor > 0.0 && aIsNode(rConstraint.msForName))
            aHeightOfWidth[rConstraint.msForName] = rConstraint.mfFactor;
    double fAspect(0.0);
    for (const ShapePtr& rNode : aNodes)
    {
        const auto aFound = aHeightOfWidth.find(rNode->getInternalName());
        if (aFound != aHeightOfWidth.end())
            fAspect = std::max(fAspect, aFound->second);
    }
    if (fAspect <= 0.0)
        fAspect = 0.6;

    // What the snake states for a transition, by its name or for every sibTrans, as a part of a
    // node: its width and its height, both kept as parts of the node width.
    std::map<OUString, std::map<sal_Int32, double>> aTransitionSize;
    for (const Constraint& rConstraint : rConstraints)
    {
        if ((rConstraint.mnType != XML_w && rConstraint.mnType != XML_h)
            || (rConstraint.mnRefType != XML_w && rConstraint.mnRefType != XML_h)
            || rConstraint.mfFactor <= 0.0 || aIsNode(rConstraint.msForName)
            || (!rConstraint.msRefForName.isEmpty() && !aIsNode(rConstraint.msRefForName)))
            continue;
        const double fOfNodeWidth(rConstraint.mnRefType == XML_w ? rConstraint.mfFactor
                                                                 : rConstraint.mfFactor * fAspect);
        for (const ShapePtr& rTransition : aTransitions)
            if (rConstraint.msForName == rTransition->getInternalName()
                || (rConstraint.msForName.isEmpty() && rConstraint.mnPointType == XML_sibTrans))
                aTransitionSize[rTransition->getInternalName()][rConstraint.mnType] = fOfNodeWidth;
    }

    // sp, the gap between two lines: a part of a node's width, or of a transition's width.
    double fBetweenLines(0.0);
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnType != XML_sp || !rConstraint.msForName.isEmpty()
            || rConstraint.mnRefType != XML_w || rConstraint.mfFactor <= 0.0)
            continue;
        if (rConstraint.msRefForName.isEmpty() || aIsNode(rConstraint.msRefForName))
            fBetweenLines = rConstraint.mfFactor;
        else
        {
            const auto aFound = aTransitionSize.find(rConstraint.msRefForName);
            if (aFound != aTransitionSize.end() && aFound->second.count(XML_w))
                fBetweenLines = rConstraint.mfFactor * aFound->second.at(XML_w);
        }
    }

    // The gap along a line is what the first transition states the way of the flow, its width
    // in a row and its height in a column, and sp where it states nothing.
    double fAlong(fBetweenLines);
    {
        const auto aFound = aTransitionSize.find(aTransitions.front()->getInternalName());
        const sal_Int32 nWay(bRows ? XML_w : XML_h);
        if (aFound != aTransitionSize.end() && aFound->second.count(nWay))
            fAlong = aFound->second.at(nWay);
    }

    // The grid, in node widths: a line holds nPerLine nodes with a gap between each two, and the
    // lines stand sp apart.
    const auto aExtent = [&](sal_Int32 nPerLine, double& rAcross, double& rDown) {
        const sal_Int32 nLines((nCount + nPerLine - 1) / nPerLine);
        const double fLine(nPerLine + (nPerLine - 1) * fAlong);
        if (bRows)
        {
            rAcross = fLine;
            rDown = nLines * fAspect + (nLines - 1) * fBetweenLines;
        }
        else
        {
            rAcross = nLines + (nLines - 1) * fBetweenLines;
            rDown = nPerLine * fAspect + (nPerLine - 1) * fAlong;
        }
        return nLines;
    };
    const auto aUnitFor = [&](sal_Int32 nPerLine) {
        double fAcross(1.0), fDown(1.0);
        aExtent(nPerLine, fAcross, fDown);
        return std::min(rShape->getSize().Width / fAcross, rShape->getSize().Height / fDown);
    };

    sal_Int32 nPerLine(1);
    if (nBreak == XML_fixed && nBreakAt >= 1)
        nPerLine = std::min(nCount, nBreakAt);
    else if (nBreak == XML_bal)
        nPerLine = static_cast<sal_Int32>(std::ceil(std::sqrt(static_cast<double>(nCount))));
    else
    {
        double fBest(0.0);
        for (sal_Int32 nTry = 1; nTry <= nCount; ++nTry)
        {
            const double fUnit(aUnitFor(nTry));
            if (fUnit > fBest)
            {
                fBest = fUnit;
                nPerLine = nTry;
            }
        }
    }

    double fAcross(1.0), fDown(1.0);
    aExtent(nPerLine, fAcross, fDown);
    const double fUnit(aUnitFor(nPerLine));
    const double fLeft((rShape->getSize().Width - fAcross * fUnit) / 2.0);
    const double fTop((rShape->getSize().Height - fDown * fUnit) / 2.0);

    // Where each node stands, in node widths from the top left of the grid.
    struct Cell
    {
        double fX, fY, fWidth, fHeight;
    };
    std::vector<Cell> aCells(nCount);
    for (sal_Int32 nNode = 0; nNode < nCount; ++nNode)
    {
        const sal_Int32 nLine(nNode / nPerLine);
        sal_Int32 nPlace(nNode % nPerLine);
        if (bBack && (nLine % 2) == 1)
            nPlace = nPerLine - 1 - nPlace;
        Cell& rCell(aCells[nNode]);
        rCell.fWidth = 1.0;
        rCell.fHeight = fAspect;
        if (bRows)
        {
            rCell.fX = nPlace * (1.0 + fAlong);
            rCell.fY = nLine * (fAspect + fBetweenLines);
        }
        else
        {
            rCell.fX = nLine * (1.0 + fBetweenLines);
            rCell.fY = nPlace * (fAspect + fAlong);
        }
        if (nGrowDir == XML_tR || nGrowDir == XML_bR)
            rCell.fX = fAcross - rCell.fX - 1.0;
        if (nGrowDir == XML_bL || nGrowDir == XML_bR)
            rCell.fY = fDown - rCell.fY - fAspect;
    }

    const auto aPlace = [&](const ShapePtr& rChild, const Cell& rCell) {
        const awt::Point aAt(static_cast<sal_Int32>(fLeft + rCell.fX * fUnit),
                             static_cast<sal_Int32>(fTop + rCell.fY * fUnit));
        const awt::Size aSize(static_cast<sal_Int32>(rCell.fWidth * fUnit),
                              static_cast<sal_Int32>(rCell.fHeight * fUnit));
        rChild->setPosition(aAt);
        rChild->setSize(aSize);
        rChild->setChildSize(aSize);
    };
    for (sal_Int32 nNode = 0; nNode < nCount; ++nNode)
        aPlace(aNodes[nNode], aCells[nNode]);

    // A transition joins the node before it and the one after it. In one line it takes the gap
    // between the two; at a turn it takes the gap between the two lines, under or beside the node
    // it leaves where the next line runs back, and from the middle of the one node to the middle
    // of the other where the next line runs the same way. It is turned the way it runs, and a
    // triangle, which points up by itself, a quarter further.
    for (size_t nTransition = 0; nTransition < aTransitions.size(); ++nTransition)
    {
        const ShapePtr& rTransition(aTransitions[nTransition]);
        const sal_Int32 nFrom(std::min<sal_Int32>(nTransition, nCount - 1));
        const sal_Int32 nTo(std::min<sal_Int32>(nTransition + 1, nCount - 1));
        const Cell& rFrom(aCells[nFrom]);
        const Cell& rTo(aCells[nTo]);
        Cell aCell{ rFrom.fX + 1.0, rFrom.fY, fAlong, fAspect };
        double fAngle(0.0);
        // A connector drawn as a line, dim 1D, is a tenth of an inch thick along a line of nodes,
        // 91440 EMU, whatever height the layout states for the transition; at a turn it keeps
        // the room of the gap it goes round.
        const auto aNode = aLayoutNodes.find(rTransition->getInternalName());
        const bool bLine(aNode != aLayoutNodes.end() && aNode->second
                         && drawsALine(*aNode->second));
        const double fLineThickness(91440.0);
        if (nTo == nFrom)
        {
            // one transition more than there are gaps: it trails the last node the way of the flow
            if (!bRows)
                aCell = Cell{ rFrom.fX, rFrom.fY + fAspect, 1.0, fAlong };
        }
        else if (bRows && std::abs(rFrom.fY - rTo.fY) < 0.001)
        {
            aCell = Cell{ std::min(rFrom.fX, rTo.fX) + 1.0, rFrom.fY, fAlong, fAspect };
            fAngle = rTo.fX > rFrom.fX ? 0.0 : 180.0;
            if (bLine)
            {
                // a line along the row is as thick as a line is, on the middle of the nodes
                aCell.fHeight = fLineThickness / fUnit;
                aCell.fY = rFrom.fY + fAspect / 2.0 - aCell.fHeight / 2.0;
            }
        }
        else if (!bRows && std::abs(rFrom.fX - rTo.fX) < 0.001)
        {
            aCell = Cell{ rFrom.fX, std::min(rFrom.fY, rTo.fY) + fAspect, 1.0, fAlong };
            fAngle = rTo.fY > rFrom.fY ? 90.0 : 270.0;
            if (bLine)
            {
                aCell.fWidth = fLineThickness / fUnit;
                aCell.fX = rFrom.fX + 0.5 - aCell.fWidth / 2.0;
            }
        }
        else if (bRows)
        {
            const double fBetween(std::min(rFrom.fY, rTo.fY) + fAspect);
            if (std::abs(rFrom.fX - rTo.fX) < 0.001)
            {
                // the same size as one along the line, turned, in the middle of the gap
                aCell = Cell{ rFrom.fX + 0.5 - fAlong / 2.0,
                              fBetween + fBetweenLines / 2.0 - fAspect / 2.0, fAlong, fAspect };
                fAngle = rTo.fY > rFrom.fY ? 90.0 : 270.0;
            }
            else
                aCell = Cell{ std::min(rFrom.fX, rTo.fX) + 0.5, fBetween,
                              std::abs(rTo.fX - rFrom.fX), fBetweenLines };
        }
        else
        {
            const double fBetween(std::min(rFrom.fX, rTo.fX) + 1.0);
            if (std::abs(rFrom.fY - rTo.fY) < 0.001)
            {
                // the same size as one along the column, turned, in the middle of the gap
                aCell = Cell{ fBetween + fBetweenLines / 2.0 - 0.5,
                              rFrom.fY + fAspect / 2.0 - fAlong / 2.0, 1.0, fAlong };
                fAngle = rTo.fX > rFrom.fX ? 0.0 : 180.0;
            }
            else
                aCell = Cell{ fBetween, std::min(rFrom.fY, rTo.fY) + fAspect / 2.0,
                              fBetweenLines, std::abs(rTo.fY - rFrom.fY) };
        }
        aPlace(rTransition, aCell);
        if (rTransition->getCustomShapeProperties()->getShapePresetType() == XML_triangle)
            fAngle += 90.0;
        rTransition->setRotation(static_cast<sal_Int32>(std::fmod(fAngle, 360.0) * PER_DEGREE));
    }

    return true;
}

// A root beside its branch, hierAlign lCtrCh and the like, laid out as a whole. The drawing sizes
// every shape from the constraints, the root's shape as tall as the root's room where it says h
// refType=h and as wide as its proportions say, the shapes in the rows below after their own, and
// then scales the whole hierarchy so that it stands in the room. Which of width and height binds
// falls out of that, and it is not the same for every file. The gap between the root's shape and
// its branch is sp, a part of the shape's width, and the rows stand sibSp apart, a part of its
// height. True when it laid the root out, false for a root that is not beside its branch or that
// states no proportions for its shape.
bool AlgAtom::layoutSidewaysRoot(const ShapePtr& rShape,
                                 const std::vector<Constraint>& rConstraints)
{
    const sal_Int32 nHierAlign(maMap.count(XML_hierAlign) ? maMap.find(XML_hierAlign)->second : 0);
    const bool bLeft(nHierAlign == XML_lCtrCh || nHierAlign == XML_lT || nHierAlign == XML_lB);
    const bool bRight(nHierAlign == XML_rCtrCh || nHierAlign == XML_rT || nHierAlign == XML_rB);
    if (!bLeft && !bRight)
        return false;

    // the shape of the root, the first child that is no connector, and its branch, the next
    ShapePtr pNode, pBranch;
    for (const ShapePtr& pChild : rShape->getChildren())
    {
        if (pChild->getSubType() == XML_conn)
            continue;
        if (!pNode)
            pNode = pChild;
        else if (!pBranch)
            pBranch = pChild;
    }
    if (!pNode)
        return false;

    const awt::Size aRoom(rShape->getSize());

    // The natural sizes, before the fit: a shape's height and width as stated, against the room
    // or against another shape by name, and where one of the two is missing its proportions give
    // it from the other. A shape that states neither is not this case.
    std::map<OUString, awt::Size> aKnown;
    const auto aNaturalSize = [&](const ShapePtr& pShape, awt::Size& rOut) {
        const OUString& rName(pShape->getInternalName());
        double fWidth(0.0), fHeight(0.0);
        const bool bWidth(readStatedSize(rConstraints, rName, XML_w, aKnown, aRoom, fWidth));
        const bool bHeight(readStatedSize(rConstraints, rName, XML_h, aKnown, aRoom, fHeight));
        const double fHeightOfWidth(readHeightOfWidthFactorOf(rConstraints, rName));
        if (!bWidth && bHeight && fHeightOfWidth > 0.0)
            fWidth = fHeight / fHeightOfWidth;
        else if (bWidth && !bHeight && fHeightOfWidth > 0.0)
            fHeight = fWidth * fHeightOfWidth;
        if (fWidth <= 0.0 || fHeight <= 0.0)
            return false;
        rOut = awt::Size(static_cast<sal_Int32>(fWidth), static_cast<sal_Int32>(fHeight));
        aKnown[rName] = rOut;
        return true;
    };

    awt::Size aNode;
    if (!aNaturalSize(pNode, aNode))
        return false;

    // the rows of the branch: each is a root of its own with a shape of its own, or a shape
    std::vector<ShapePtr> aRows;
    std::vector<awt::Size> aRowSizes;
    if (pBranch)
        for (const ShapePtr& pRow : pBranch->getChildren())
        {
            if (pRow->getSubType() == XML_conn)
                continue;
            ShapePtr pRowNode(pRow);
            for (const ShapePtr& pInside : pRow->getChildren())
                if (pInside->getSubType() != XML_conn)
                {
                    pRowNode = pInside;
                    break;
                }
            awt::Size aRowNode;
            if (!aNaturalSize(pRowNode, aRowNode))
                return false;
            aRows.push_back(pRow);
            aRowSizes.push_back(aRowNode);
        }

    const double fGap(readSpacingFactor(rConstraints, XML_w, 0.4) * aNode.Width);
    const double fRowGap(readSpacingFactor(rConstraints, XML_h, 0.15) * aNode.Height);
    double fBranchWidth(0.0), fBranchHeight(0.0);
    for (size_t nRow = 0; nRow < aRows.size(); ++nRow)
    {
        fBranchWidth = std::max<double>(fBranchWidth, aRowSizes[nRow].Width);
        fBranchHeight += aRowSizes[nRow].Height + (nRow > 0 ? fRowGap : 0.0);
    }
    const double fWholeWidth(aNode.Width + (aRows.empty() ? 0.0 : fGap + fBranchWidth));
    const double fWholeHeight(std::max<double>(aNode.Height, fBranchHeight));
    if (fWholeWidth <= 0.0 || fWholeHeight <= 0.0)
        return false;
    const double fScale(std::min(aRoom.Width / fWholeWidth, aRoom.Height / fWholeHeight));

    // the root's shape at the side it is aligned to, and in the middle of the height of the
    // branch, or at its top or bottom, as hierAlign says
    const awt::Size aNodeSize(static_cast<sal_Int32>(aNode.Width * fScale),
                              static_cast<sal_Int32>(aNode.Height * fScale));
    const sal_Int32 nBranchHeight(static_cast<sal_Int32>(fBranchHeight * fScale));
    const sal_Int32 nWholeHeight(std::max(aNodeSize.Height, nBranchHeight));
    const sal_Int32 nTop((aRoom.Height - nWholeHeight) / 2);
    sal_Int32 nNodeTop(nTop + (nWholeHeight - aNodeSize.Height) / 2);
    if (nHierAlign == XML_lT || nHierAlign == XML_rT)
        nNodeTop = nTop;
    else if (nHierAlign == XML_lB || nHierAlign == XML_rB)
        nNodeTop = nTop + nWholeHeight - aNodeSize.Height;
    const sal_Int32 nBranchWidth(static_cast<sal_Int32>(
        std::max(1.0, aRoom.Width - (aNode.Width + fGap) * fScale)));
    const sal_Int32 nNodeLeft(bLeft ? 0 : aRoom.Width - aNodeSize.Width);
    pNode->setPosition(awt::Point(nNodeLeft, nNodeTop));
    pNode->setSize(aNodeSize);
    pNode->setChildSize(aNodeSize);

    if (pBranch)
    {
        const sal_Int32 nBranchLeft(bLeft ? static_cast<sal_Int32>((aNode.Width + fGap) * fScale)
                                          : 0);
        const awt::Size aBranchSize(nBranchWidth, std::max<sal_Int32>(1, nBranchHeight));
        pBranch->setPosition(awt::Point(nBranchLeft, nTop + (nWholeHeight - nBranchHeight) / 2));
        pBranch->setSize(aBranchSize);
        pBranch->setChildSize(aBranchSize);
    }

    // a connector of the root lies in the gap, from the shape to the branch, as high as the shape
    for (const ShapePtr& pChild : rShape->getChildren())
    {
        if (pChild->getSubType() != XML_conn)
            continue;
        const awt::Size aLine(std::max<sal_Int32>(1, static_cast<sal_Int32>(fGap * fScale)),
                              aNodeSize.Height);
        pChild->setPosition(awt::Point(bLeft ? aNodeSize.Width : nBranchWidth, nNodeTop));
        pChild->setSize(aLine);
        pChild->setChildSize(aLine);
    }
    return true;
}

// The type of the algorithm below the layout node, through a choose, or 0 where there is none.
static sal_Int32 algorithmTypeOf(const LayoutAtom& rAtom)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
            return pAlg->getType();
        const sal_Int32 nBelow(algorithmTypeOf(*pChild));
        if (nBelow != 0)
            return nBelow;
    }
    return 0;
}

// hierAlign of the hierarchy root algorithm below the layout node, through a choose, or 0 where
// there is none or it states none.
static sal_Int32 hierarchyAlignOf(const LayoutAtom& rAtom)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (dynamic_cast<const LayoutNode*>(pChild.get()))
            continue;
        if (const AlgAtom* pAlg = dynamic_cast<const AlgAtom*>(pChild.get()))
        {
            if (pAlg->getType() != XML_hierRoot)
                continue;
            const auto aAlign = pAlg->getMap().find(XML_hierAlign);
            return aAlign == pAlg->getMap().end() ? 0 : aAlign->second;
        }
        const sal_Int32 nBelow(hierarchyAlignOf(*pChild));
        if (nBelow != 0)
            return nBelow;
    }
    return 0;
}

namespace
{
// A root of a sideways hierarchy with its natural sizes, before the fit: the shape of the root,
// its branch below, and the rows of that branch, each a root again.
struct SidewaysRoot
{
    ShapePtr pRoot;
    ShapePtr pNode;
    ShapePtr pBranch;
    awt::Size aNode;
    std::vector<SidewaysRoot> aRows;
    double fGap = 0.0; // between the node and the branch
    double fRowGap = 0.0; // between two rows of the branch
    double fWidth = 0.0; // the whole, node, gap and branch
    double fHeight = 0.0;
    double fBranchHeight = 0.0;
    double fBranchWidth = 0.0;
};
}

// A gap, sp or sibSp, as the constraints state it: a part of the size of the shape it names, w
// for sp and h for sibSp, where that shape is known by name, and a part of rOwn otherwise.
static double readSidewaysGap(const std::vector<Constraint>& rConstraints, sal_Int32 nType,
                              const std::map<OUString, awt::Size>& rKnown, double fOwn,
                              double fDefault)
{
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnType != nType || rConstraint.mfFactor <= 0.0)
            continue;
        const auto aOther = rKnown.find(rConstraint.msRefForName);
        if (!rConstraint.msRefForName.isEmpty() && aOther != rKnown.end())
            return rConstraint.mfFactor
                   * (rConstraint.mnRefType == XML_w ? aOther->second.Width
                                                     : aOther->second.Height);
        if (rConstraint.mnRefType == XML_w || rConstraint.mnRefType == XML_h)
            return rConstraint.mfFactor * fOwn;
    }
    return fDefault * fOwn;
}

// The rows of a branch, each a root of its own. The shape of each row's root is sized from the
// constraints, against the room given or against a shape sized before by name, and where one of
// width and height is missing its proportions give it. False where a row states nothing usable.
static bool gatherSidewaysRows(const ShapePtr& pBranch, const std::vector<Constraint>& rConstraints,
                               const awt::Size& rRoom, std::map<OUString, awt::Size>& rKnown,
                               std::vector<SidewaysRoot>& rRows)
{
    for (const ShapePtr& pRow : pBranch->getChildren())
    {
        if (pRow->getSubType() == XML_conn)
            continue;
        SidewaysRoot aRow;
        aRow.pRoot = pRow;
        for (const ShapePtr& pInside : pRow->getChildren())
        {
            if (pInside->getSubType() == XML_conn)
                continue;
            if (!aRow.pNode)
                aRow.pNode = pInside;
            else if (!aRow.pBranch)
                aRow.pBranch = pInside;
        }
        if (!aRow.pNode)
            aRow.pNode = pRow;

        const OUString& rName(aRow.pNode->getInternalName());
        double fWidth(0.0), fHeight(0.0);
        const bool bWidth(readStatedSize(rConstraints, rName, XML_w, rKnown, rRoom, fWidth));
        const bool bHeight(readStatedSize(rConstraints, rName, XML_h, rKnown, rRoom, fHeight));
        const double fHeightOfWidth(readHeightOfWidthFactorOf(rConstraints, rName));
        if (!bWidth && bHeight && fHeightOfWidth > 0.0)
            fWidth = fHeight / fHeightOfWidth;
        else if (bWidth && !bHeight && fHeightOfWidth > 0.0)
            fHeight = fWidth * fHeightOfWidth;
        if (fWidth <= 0.0 || fHeight <= 0.0)
            return false;
        aRow.aNode = awt::Size(static_cast<sal_Int32>(fWidth), static_cast<sal_Int32>(fHeight));
        rKnown[rName] = aRow.aNode;
        if (aRow.pBranch
            && !gatherSidewaysRows(aRow.pBranch, rConstraints, rRoom, rKnown, aRow.aRows))
            return false;
        // the gaps after the rows, which may be the shapes they are stated against
        aRow.fGap = readSidewaysGap(rConstraints, XML_sp, rKnown, aRow.aNode.Width, 0.4);
        aRow.fRowGap = readSidewaysGap(rConstraints, XML_sibSp, rKnown, aRow.aNode.Height, 0.15);
        for (size_t nSub = 0; nSub < aRow.aRows.size(); ++nSub)
        {
            aRow.fBranchWidth = std::max(aRow.fBranchWidth, aRow.aRows[nSub].fWidth);
            aRow.fBranchHeight += aRow.aRows[nSub].fHeight + (nSub > 0 ? aRow.fRowGap : 0.0);
        }
        aRow.fWidth = aRow.aNode.Width + (aRow.aRows.empty() ? 0.0 : aRow.fGap + aRow.fBranchWidth);
        aRow.fHeight = std::max<double>(aRow.aNode.Height, aRow.fBranchHeight);
        rRows.push_back(aRow);
    }
    return true;
}

// Places a root and all below it at the scale found, the root's box at rAt with the size its
// natural sizes give. The node stands at the side hierAlign says and in the middle of the height
// of the branch, the branch beside it after the gap, its rows spread over the height of the root
// where they do not fill it, and a connector of the branch is the line from the middle of the
// node's side to the middle of the row's node's side.
static void placeSidewaysRoot(SmartArtDiagram& rDgm, const SidewaysRoot& rRoot,
                              const awt::Point& rAt,
                              double fScale, bool bLeft)
{
    const awt::Size aBox(static_cast<sal_Int32>(rRoot.fWidth * fScale),
                         static_cast<sal_Int32>(rRoot.fHeight * fScale));
    rRoot.pRoot->setPosition(rAt);
    rRoot.pRoot->setSize(aBox);
    rRoot.pRoot->setChildSize(aBox);
    rDgm.getLaidOutSideways().insert(rRoot.pRoot.get());

    const awt::Size aNode(static_cast<sal_Int32>(rRoot.aNode.Width * fScale),
                          static_cast<sal_Int32>(rRoot.aNode.Height * fScale));
    const sal_Int32 nNodeLeft(bLeft ? 0 : aBox.Width - aNode.Width);
    const sal_Int32 nNodeTop((aBox.Height - aNode.Height) / 2);
    if (rRoot.pNode != rRoot.pRoot)
    {
        rRoot.pNode->setPosition(awt::Point(nNodeLeft, nNodeTop));
        rRoot.pNode->setSize(aNode);
        rRoot.pNode->setChildSize(aNode);
    }
    if (!rRoot.pBranch || rRoot.aRows.empty())
        return;

    const sal_Int32 nGap(static_cast<sal_Int32>(rRoot.fGap * fScale));
    const awt::Size aBranch(std::max<sal_Int32>(1, aBox.Width - aNode.Width - nGap), aBox.Height);
    const sal_Int32 nBranchLeft(bLeft ? aNode.Width + nGap : 0);
    rRoot.pBranch->setPosition(awt::Point(nBranchLeft, 0));
    rRoot.pBranch->setSize(aBranch);
    rRoot.pBranch->setChildSize(aBranch);
    rDgm.getLaidOutSideways().insert(rRoot.pBranch.get());

    // the rows, sibSp apart, in the middle of the height of the branch where they are shorter
    double fRowsHeight(0.0);
    for (const SidewaysRoot& rRow : rRoot.aRows)
        fRowsHeight += rRow.fHeight * fScale;
    const double fRowGap(rRoot.fRowGap * fScale);
    fRowsHeight += fRowGap * (rRoot.aRows.size() - 1);
    double fY((aBranch.Height - fRowsHeight) / 2.0);
    std::vector<awt::Point> aRowMiddles;
    for (const SidewaysRoot& rRow : rRoot.aRows)
    {
        const awt::Size aRowBox(static_cast<sal_Int32>(rRow.fWidth * fScale),
                                static_cast<sal_Int32>(rRow.fHeight * fScale));
        const sal_Int32 nRowLeft(bLeft ? 0 : aBranch.Width - aRowBox.Width);
        placeSidewaysRoot(rDgm, rRow, awt::Point(nRowLeft, static_cast<sal_Int32>(fY)), fScale,
                          bLeft);
        aRowMiddles.emplace_back(bLeft ? nRowLeft : nRowLeft + aRowBox.Width,
                                 static_cast<sal_Int32>(fY) + aRowBox.Height / 2);
        fY += aRowBox.Height + fRowGap;
    }

    // the connectors of the branch, one before each row, from the node to that row's node; a
    // connector routed as a bend, connRout bend, is the elbow over the gap, across to its
    // middle, down or up to the row's middle and across again, Horizontal_Multi-level_
    // Hierarchy's lines from a level's right edge to the left edges of the level below
    const sal_Int32 nFromX(bLeft ? -nGap : aBranch.Width + nGap);
    const sal_Int32 nNodeMiddleY(nNodeTop + aNode.Height / 2);
    std::map<OUString, const LayoutNode*> aLayoutNodes;
    if (rDgm.getLayout() && rDgm.getLayout()->getNode())
        gatherLayoutNodes(*rDgm.getLayout()->getNode(), aLayoutNodes);
    size_t nRow(0);
    for (const ShapePtr& pChild : rRoot.pBranch->getChildren())
    {
        if (pChild->getSubType() != XML_conn)
            continue;
        if (nRow >= aRowMiddles.size())
            break;
        const awt::Point aStart(nFromX, nNodeMiddleY);
        const awt::Point aStop(aRowMiddles[nRow]);
        ++nRow;
        const auto aConnectorNode = aLayoutNodes.find(pChild->getInternalName());
        if (aConnectorNode != aLayoutNodes.end() && aConnectorNode->second
            && connectorRouteOf(*aConnectorNode->second) == XML_bend)
        {
            const awt::Size aElbow(std::max<sal_Int32>(1, std::abs(aStop.X - aStart.X)),
                                   std::max<sal_Int32>(1, std::abs(aStop.Y - aStart.Y)));
            pChild->setPosition(awt::Point(std::min(aStart.X, aStop.X),
                                           std::min(aStart.Y, aStop.Y)));
            pChild->setSize(aElbow);
            pChild->setChildSize(aElbow);
            // the bent connector runs from its top left corner to its bottom right one, the
            // bend in the middle of its width; it is mirrored where the row's node stands above
            // the node, and where it stands left of it
            // each connector needs custom shape properties of its own, the mirroring is kept there
            pChild->setSubType(XML_bentConnector3);
            pChild->getCustomShapeProperties()
                = std::make_shared<CustomShapeProperties>(*pChild->getCustomShapeProperties());
            auto& rProperties(*pChild->getCustomShapeProperties());
            rProperties.setShapePresetType(XML_bentConnector3);
            pChild->setFlip((aStop.X < aStart.X), (aStop.Y < aStart.Y));
            rDgm.getLaidOutSideways().insert(pChild.get());
            continue;
        }
        const double fDx(aStop.X - aStart.X), fDy(aStop.Y - aStart.Y);
        const double fWay(std::max(1.0, std::hypot(fDx, fDy)));
        const awt::Size aLine(static_cast<sal_Int32>(fWay), 36000);
        pChild->setPosition(awt::Point((aStart.X + aStop.X) / 2 - aLine.Width / 2,
                                       (aStart.Y + aStop.Y) / 2 - aLine.Height / 2));
        pChild->setSize(aLine);
        pChild->setChildSize(aLine);
        pChild->setRotation(
            static_cast<sal_Int32>(basegfx::rad2deg(atan2(fDy, fDx)) * PER_DEGREE));
        rDgm.getLaidOutSideways().insert(pChild.get());
    }
}

// A branch whose rows are roots that stand beside their branches, hierAlign lCtrCh and the like,
// is the top of a hierarchy that lies sideways. The drawing sizes every shape in it from the
// constraints, the roots' shapes as tall as their share of the branch and as wide as their
// proportions say, the shapes below after their own statements, and then scales the whole so
// that it stands in the branch. Which of width and height binds falls out of that.
bool AlgAtom::layoutSidewaysBranch(const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                                   const std::vector<Constraint>& rConstraints)
{
    const sal_Int32 nDir(maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromL);
    if (nDir != XML_fromT && nDir != XML_fromB)
        return false;

    // every row a root that lies sideways, and all to the same side
    std::map<OUString, const LayoutNode*> aLayoutNodes;
    gatherLayoutNodes(mrLayoutNode, aLayoutNodes);
    sal_Int32 nAlign(0);
    sal_Int32 nRows(0);
    for (const ShapePtr& pChild : rShape->getChildren())
    {
        if (pChild->getSubType() == XML_conn)
            continue;
        const auto aNode = aLayoutNodes.find(pChild->getInternalName());
        const sal_Int32 nOwn(aNode != aLayoutNodes.end() && aNode->second
                                 ? hierarchyAlignOf(*aNode->second)
                                 : 0);
        const bool bSideways(nOwn == XML_lCtrCh || nOwn == XML_lT || nOwn == XML_lB
                             || nOwn == XML_rCtrCh || nOwn == XML_rT || nOwn == XML_rB);
        if (!bSideways || (nAlign != 0 && nOwn != nAlign))
            return false;
        nAlign = nOwn;
        ++nRows;
    }
    if (nRows == 0)
        return false;
    const bool bLeft(nAlign == XML_lCtrCh || nAlign == XML_lT || nAlign == XML_lB);

    // A shape that states h refType=h, its height as the height of its room, is one unit high,
    // the same unit at every level, and one unit wide where it says the same of its width. The
    // scale found below turns the unit into what fits, so its size is of no consequence.
    const awt::Size aRoom(1000000, 1000000);
    std::map<OUString, awt::Size> aKnown;
    std::vector<SidewaysRoot> aRows;
    if (!gatherSidewaysRows(rShape, rConstraints, aRoom, aKnown, aRows) || aRows.empty())
        return false;

    // The roots and their branches are two columns packed each for itself: the branches stand
    // one below the other with the branch's row gap between two of them, each root in the
    // middle of its branch's height, and a root without a branch comes sibSp below the root
    // before it, beside whatever branch stands there. A root that the roots' column pushes
    // down, sibSp below the one before, takes its branch down with it. Horizontal_Hierarchy's
    // Three has no children and stands beside Two's last child, and its Four is in the middle
    // of its two children and, at that, sibSp below Three: the whole is the seven children with
    // their six gaps high, 7.9 nodes, where a row for Three on its own made it 9.05.
    double fWidth(0.0);
    const double fBetweenRoots(
        readSidewaysGap(rConstraints, XML_sibSp, aKnown, aRows.front().aNode.Height, 0.15));
    std::vector<double> aTops;
    double fBranchesBottom(0.0);
    double fRootsBottom(0.0);
    double fHeight(0.0);
    for (size_t nRow = 0; nRow < aRows.size(); ++nRow)
    {
        const SidewaysRoot& rRow(aRows[nRow]);
        fWidth = std::max(fWidth, rRow.fWidth);
        const bool bBranch(rRow.pBranch && !rRow.aRows.empty() && rRow.fBranchHeight > 0.0);
        double fRootTop(0.0);
        double fTop(0.0);
        if (bBranch)
        {
            double fBranchTop(nRow > 0 ? fBranchesBottom + rRow.fRowGap : 0.0);
            fRootTop = fBranchTop + (rRow.fBranchHeight - rRow.aNode.Height) / 2.0;
            const double fLeast(nRow > 0 ? fRootsBottom + fBetweenRoots : 0.0);
            if (fRootTop < fLeast)
            {
                fBranchTop += fLeast - fRootTop;
                fRootTop = fLeast;
            }
            fTop = std::min(fBranchTop, fRootTop);
            fBranchesBottom = fBranchTop + rRow.fBranchHeight;
        }
        else
        {
            fRootTop = nRow > 0 ? fRootsBottom + fBetweenRoots : 0.0;
            fTop = fRootTop;
        }
        fRootsBottom = fRootTop + rRow.aNode.Height;
        aTops.push_back(fTop);
        fHeight = std::max(fHeight, fTop + rRow.fHeight);
    }
    if (fWidth <= 0.0 || fHeight <= 0.0)
        return false;
    const double fScale(
        std::min(rShape->getSize().Width / fWidth, rShape->getSize().Height / fHeight));

    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    rMutable.getLaidOutSideways().insert(rShape.get());
    const double fY0((rShape->getSize().Height - fHeight * fScale) / 2.0);
    for (size_t nRow = 0; nRow < aRows.size(); ++nRow)
    {
        const SidewaysRoot& rRow(aRows[nRow]);
        const sal_Int32 nRowWidth(static_cast<sal_Int32>(rRow.fWidth * fScale));
        const sal_Int32 nRowLeft(bLeft ? 0 : rShape->getSize().Width - nRowWidth);
        placeSidewaysRoot(rMutable, rRow,
                          awt::Point(nRowLeft, static_cast<sal_Int32>(fY0 + aTops[nRow] * fScale)),
                          fScale, bLeft);
    }
    return true;
}

namespace
{
// A node of an upright hierarchy with its natural sizes, before the fit: the root group, the cell
// that draws the node, the branch below and the nodes in it. The cell's middle is the origin of
// the node; the children stand at aOffsets from it, and the contour is, level by level down from
// the node, how far the subtree reaches left and right of that origin.
struct UprightNode
{
    ShapePtr pRoot;
    ShapePtr pCell;
    ShapePtr pBranch;
    awt::Size aCell;
    std::vector<UprightNode> aChildren;
    std::vector<double> aOffsets;
    std::vector<std::pair<double, double>> aContour;
};

// The nodes of a branch, each a root of its own with the cell sized from the constraints.
bool gatherUprightNodes(const SmartArtDiagram& rDgm, const ShapePtr& pBranch,
                        const std::vector<Constraint>& rConstraints, const awt::Size& rRoom,
                        std::map<OUString, awt::Size>& rKnown,
                        const std::map<OUString, const LayoutNode*>& rLayoutNodes,
                        const std::map<const Shape*, rtl::Reference<svx::diagram::Point>>& rPoints,
                        std::vector<UprightNode>& rNodes)
{
    for (const ShapePtr& pRow : pBranch->getChildren())
    {
        if (pRow->getSubType() == XML_conn)
            continue;
        UprightNode aNode;
        aNode.pRoot = pRow;
        for (const ShapePtr& pInside : pRow->getChildren())
        {
            if (pInside->getSubType() == XML_conn)
                continue;
            if (!aNode.pCell)
                aNode.pCell = pInside;
            else if (!aNode.pBranch)
                aNode.pBranch = pInside;
        }
        if (!aNode.pCell)
            aNode.pCell = pRow;

        const OUString& rName(aNode.pCell->getInternalName());
        double fWidth(0.0), fHeight(0.0);
        const bool bWidth(readStatedSize(rConstraints, rName, XML_w, rKnown, rRoom, fWidth));
        const bool bHeight(readStatedSize(rConstraints, rName, XML_h, rKnown, rRoom, fHeight));
        const double fHeightOfWidth(readHeightOfWidthFactorOf(rConstraints, rName));
        if (!bWidth && bHeight && fHeightOfWidth > 0.0)
            fWidth = fHeight / fHeightOfWidth;
        else if (bWidth && !bHeight && fHeightOfWidth > 0.0)
            fHeight = fWidth * fHeightOfWidth;
        if (fWidth <= 0.0 || fHeight <= 0.0)
            return false;
        aNode.aCell = awt::Size(static_cast<sal_Int32>(fWidth), static_cast<sal_Int32>(fHeight));
        rKnown[rName] = aNode.aCell;
        bool bBranchHasNodes(false);
        if (aNode.pBranch)
            for (const ShapePtr& pBelow : aNode.pBranch->getChildren())
                bBranchHasNodes = bBranchHasNodes || pBelow->getSubType() != XML_conn;
        if (bBranchHasNodes)
        {
            // A branch below that hangs its children in a column is not this kind of hierarchy.
            // A choose may stand in front of the branch's algorithm, one branch of it for each
            // way the node's children hang, so the algorithm is asked for the way the layout
            // read it for this very node: against the presentation point the branch draws,
            // which the shape's own id does not name, that names the data node.
            const auto aBranchNode = rLayoutNodes.find(aNode.pBranch->getInternalName());
            if (aBranchNode == rLayoutNodes.end() || !aBranchNode->second)
                return false;
            const auto aPoint = rPoints.find(aNode.pBranch.get());
            const rtl::Reference<svx::diagram::Point> xBranchPoint(
                aPoint != rPoints.end() ? aPoint->second : rtl::Reference<svx::diagram::Point>());
            const AlgAtom* pBranchAlg(algorithmOf(rDgm, *aBranchNode->second, xBranchPoint));
            if (!pBranchAlg || pBranchAlg->getType() != XML_hierChild)
                return false;
            const AlgAtom::ParamMap& rBranchMap(pBranchAlg->getMap());
            const auto aDir = rBranchMap.find(XML_linDir);
            const sal_Int32 nBranchDir(aDir == rBranchMap.end() ? XML_fromL : aDir->second);
            if ((nBranchDir != XML_fromL && nBranchDir != XML_fromR)
                || rBranchMap.count(XML_secLinDir))
                return false;
            if (!gatherUprightNodes(rDgm, aNode.pBranch, rConstraints, rRoom, rKnown,
                                    rLayoutNodes, rPoints, aNode.aChildren))
                return false;
        }
        rNodes.push_back(aNode);
    }
    return true;
}

// Packs rNodes side by side, each as close to the one before as any level of the two allows with
// the gap between, and returns where each stands against the first, with the contour of them all.
void packUpright(std::vector<UprightNode>& rNodes, double fGap, std::vector<double>& rOffsets,
                 std::vector<std::pair<double, double>>& rContour)
{
    rOffsets.clear();
    rContour.clear();
    for (UprightNode& rNode : rNodes)
    {
        double fShift(0.0);
        if (!rContour.empty())
        {
            fShift = -std::numeric_limits<double>::max();
            for (size_t nLevel = 0; nLevel < rContour.size() && nLevel < rNode.aContour.size();
                 ++nLevel)
                fShift = std::max(fShift, rContour[nLevel].second + fGap
                                              - rNode.aContour[nLevel].first);
        }
        rOffsets.push_back(fShift);
        for (size_t nLevel = 0; nLevel < rNode.aContour.size(); ++nLevel)
        {
            const std::pair<double, double> aReach(rNode.aContour[nLevel].first + fShift,
                                                   rNode.aContour[nLevel].second + fShift);
            if (nLevel < rContour.size())
            {
                rContour[nLevel].first = std::min(rContour[nLevel].first, aReach.first);
                rContour[nLevel].second = std::max(rContour[nLevel].second, aReach.second);
            }
            else
                rContour.push_back(aReach);
        }
    }
}

// Works out the contour of a node and where its children stand: the children packed, the node
// in the middle above them.
void shapeUpright(UprightNode& rNode, double fGap)
{
    for (UprightNode& rChild : rNode.aChildren)
        shapeUpright(rChild, fGap);
    rNode.aContour.clear();
    rNode.aContour.emplace_back(-rNode.aCell.Width / 2.0, rNode.aCell.Width / 2.0);
    if (rNode.aChildren.empty())
        return;
    std::vector<std::pair<double, double>> aBelow;
    packUpright(rNode.aChildren, fGap, rNode.aOffsets, aBelow);
    const double fMiddle((rNode.aOffsets.front() + rNode.aOffsets.back()) / 2.0);
    for (double& rOffset : rNode.aOffsets)
        rOffset -= fMiddle;
    for (const auto& rReach : aBelow)
        rNode.aContour.emplace_back(rReach.first - fMiddle, rReach.second - fMiddle);
}

// The depth of a node, in levels, itself included.
size_t uprightDepth(const UprightNode& rNode)
{
    size_t nBelow(0);
    for (const UprightNode& rChild : rNode.aChildren)
        nBelow = std::max(nBelow, uprightDepth(rChild));
    return nBelow + 1;
}

// Places a node with everything below it. fMiddle is where the middle of its cell stands and
// fTop where its cell begins, both against the box the node's root group stands in, in natural
// units; fScale turns those into what the shapes get. The root group takes the box of its
// subtree, the cell stands in it, the branch below the cell after the level gap, and each
// connector in the branch is the elbow from the bottom of the cell down to the top of a child.
void placeUpright(SmartArtDiagram& rDgm, const UprightNode& rNode, double fMiddle, double fTop,
                  double fScale, double fLevelGap, double fLevelHeight)
{
    double fLeft(fMiddle), fRight(fMiddle);
    for (const auto& rReach : rNode.aContour)
    {
        fLeft = std::min(fLeft, fMiddle + rReach.first);
        fRight = std::max(fRight, fMiddle + rReach.second);
    }
    const size_t nDepth(uprightDepth(rNode));
    const double fHeight(nDepth * fLevelHeight + (nDepth - 1) * fLevelGap);
    const awt::Point aBoxAt(static_cast<sal_Int32>(fLeft * fScale),
                            static_cast<sal_Int32>(fTop * fScale));
    const awt::Size aBox(std::max<sal_Int32>(1, static_cast<sal_Int32>((fRight - fLeft) * fScale)),
                         std::max<sal_Int32>(1, static_cast<sal_Int32>(fHeight * fScale)));
    rNode.pRoot->setPosition(aBoxAt);
    rNode.pRoot->setSize(aBox);
    rNode.pRoot->setChildSize(aBox);
    rDgm.getLaidOutSideways().insert(rNode.pRoot.get());

    const awt::Size aCell(static_cast<sal_Int32>(rNode.aCell.Width * fScale),
                          static_cast<sal_Int32>(rNode.aCell.Height * fScale));
    const sal_Int32 nCellLeft(static_cast<sal_Int32>((fMiddle - fLeft) * fScale) - aCell.Width / 2);
    if (rNode.pCell != rNode.pRoot)
    {
        rNode.pCell->setPosition(awt::Point(nCellLeft, 0));
        rNode.pCell->setSize(aCell);
        rNode.pCell->setChildSize(aCell);
    }
    if (!rNode.pBranch || rNode.aChildren.empty())
        return;

    const sal_Int32 nBranchTop(static_cast<sal_Int32>((fLevelHeight + fLevelGap) * fScale));
    const awt::Size aBranch(aBox.Width, std::max<sal_Int32>(1, aBox.Height - nBranchTop));
    rNode.pBranch->setPosition(awt::Point(0, nBranchTop));
    rNode.pBranch->setSize(aBranch);
    rNode.pBranch->setChildSize(aBranch);
    rDgm.getLaidOutSideways().insert(rNode.pBranch.get());

    // the children against the branch, whose left is fLeft and whose top is the next level
    std::vector<sal_Int32> aChildMiddles;
    for (size_t nChild = 0; nChild < rNode.aChildren.size(); ++nChild)
    {
        const double fChildMiddle(fMiddle + rNode.aOffsets[nChild] - fLeft);
        placeUpright(rDgm, rNode.aChildren[nChild], fChildMiddle, 0.0, fScale, fLevelGap,
                     fLevelHeight);
        aChildMiddles.push_back(static_cast<sal_Int32>(fChildMiddle * fScale));
    }

    // the connectors, one before each child: an elbow from the bottom middle of the cell to the
    // top middle of the child, at least a tenth of an inch wide where the two stand in line
    const sal_Int32 nCellMiddle(static_cast<sal_Int32>((fMiddle - fLeft) * fScale));
    const sal_Int32 nGap(static_cast<sal_Int32>(fLevelGap * fScale));
    size_t nChild(0);
    for (const ShapePtr& pChild : rNode.pBranch->getChildren())
    {
        if (pChild->getSubType() != XML_conn)
            continue;
        if (nChild >= aChildMiddles.size())
            break;
        const sal_Int32 nTo(aChildMiddles[nChild++]);
        const sal_Int32 nWidth(std::max<sal_Int32>(91440, std::abs(nTo - nCellMiddle)));
        const awt::Size aElbow(nWidth, std::max<sal_Int32>(1, nGap));
        pChild->setPosition(awt::Point(std::min(nCellMiddle, nTo) - (nWidth == 91440 ? 45720 : 0),
                                       -nGap));
        pChild->setSize(aElbow);
        pChild->setChildSize(aElbow);

        // A bent connector draws the elbow once its corner sits on its own left edge, which is
        // where an adjustment of zero puts it: down from the top left, then across to the bottom
        // right. Where the child stands left of the cell the connector is mirrored.
        // Each connector needs custom shape properties of its own, the mirroring is kept there.
        pChild->setSubType(XML_bentConnector3);
        pChild->getCustomShapeProperties()
            = std::make_shared<CustomShapeProperties>(*pChild->getCustomShapeProperties());
        auto& rProperties(*pChild->getCustomShapeProperties());
        rProperties.setShapePresetType(XML_bentConnector3);
        if (rProperties.getAdjustmentGuideList().GetCustomShapeGuideValue(u"adj1"_ustr) < 0)
            rProperties.getAdjustmentGuideList().push_back({ u"adj1"_ustr, u"0"_ustr });
        pChild->setFlip(nTo < nCellMiddle, false);
        rDgm.getLaidOutSideways().insert(pChild.get());
    }
}
}

// A row of roots that stand above their branches is the top of an upright hierarchy, an
// organisation chart. The drawing sizes every cell from the constraints, packs the children of
// a node side by side, each as close to the one before as any level of the two allows with sibSp
// between, puts the node in the middle above them, does the same with the roots, and then scales
// the whole so that it stands in the row, in the middle of it both ways. The roots' cells are
// each one unit wide where they say w refType=w, the scale absorbing the unit.
bool AlgAtom::layoutUprightBranch(const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                                  const std::vector<Constraint>& rConstraints)
{
    const sal_Int32 nDir(maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromL);
    if ((nDir != XML_fromL && nDir != XML_fromR) || maMap.count(XML_secLinDir))
        return false;

    // every row a root that stands above its branch
    std::map<OUString, const LayoutNode*> aLayoutNodes;
    gatherLayoutNodes(mrLayoutNode, aLayoutNodes);
    sal_Int32 nRows(0);
    for (const ShapePtr& pChild : rShape->getChildren())
    {
        if (pChild->getSubType() == XML_conn)
            continue;
        // a root is what the hierarchy root algorithm draws; the shape carries the type it is
        // drawn as, so the layout node is asked
        const auto aNode = aLayoutNodes.find(pChild->getInternalName());
        if (aNode == aLayoutNodes.end() || !aNode->second
            || algorithmTypeOf(*aNode->second) != XML_hierRoot
            || containsDataNodeType(pChild, XML_asst))
            return false;
        const sal_Int32 nAlign(hierarchyAlignOf(*aNode->second));
        if (nAlign != 0 && nAlign != XML_tL && nAlign != XML_tCtrCh && nAlign != XML_tR)
            return false;
        ++nRows;
    }
    if (nRows == 0)
        return false;

    // the presentation point each shape draws, for the chooses of the branches below
    std::map<const Shape*, rtl::Reference<svx::diagram::Point>> aPoints;
    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
        if (rEntry.second)
            aPoints[rEntry.second.get()] = rEntry.first;

    const awt::Size aRoom(1000000, 1000000);
    std::map<OUString, awt::Size> aKnown;
    std::vector<UprightNode> aRoots;
    if (!gatherUprightNodes(rDgm, rShape, rConstraints, aRoom, aKnown, aLayoutNodes, aPoints,
                            aRoots)
        || aRoots.empty())
        return false;

    // the cells of one hierarchy are all as high as the first, and the gaps are stated against
    // that cell
    const double fLevelHeight(aRoots.front().aCell.Height);
    const double fGap(readSidewaysGap(rConstraints, XML_sibSp, aKnown, aRoots.front().aCell.Width,
                                      0.1));
    double fLevelGap(readSidewaysGap(rConstraints, XML_sp, aKnown, fLevelHeight, 0.25));

    // Where the branch's connectors name the shapes they join, from the bottom middle of the one
    // to the top middle of the other, sp is the way between those two shapes and not between
    // the cells: Circle_Picture_Hierarchy's elbows run from the picture of a node, 0.1 of the
    // cell below its top and 0.8 high, to the picture of its child, 0.1 below that cell's top,
    // and the cells stand 0.25 less those two margins apart, 561554 from one top to the next
    // for cells of 533796.
    for (const auto& rEntry : aLayoutNodes)
    {
        if (!rEntry.second)
            continue;
        std::vector<const AlgAtom*> aAlgorithms;
        gatherOwnAlgorithms(*rEntry.second, aAlgorithms);
        const AlgAtom* pConn(nullptr);
        for (const AlgAtom* pAlg : aAlgorithms)
        {
            const AlgAtom::ParamMap& rMap(pAlg->getMap());
            const auto aRoute = rMap.find(XML_connRout);
            const auto aBegin = rMap.find(XML_begPts);
            const auto aEnd = rMap.find(XML_endPts);
            if (pAlg->getType() == XML_conn && aRoute != rMap.end() && aRoute->second == XML_bend
                && aBegin != rMap.end() && aBegin->second == XML_bCtr && aEnd != rMap.end()
                && aEnd->second == XML_tCtr && !pAlg->getNamedParam(XML_srcNode).isEmpty()
                && !pAlg->getNamedParam(XML_dstNode).isEmpty())
                pConn = pAlg;
        }
        if (!pConn)
            continue;
        // the top and the height of a named shape as parts of the cell it stands in
        const auto aPartsOf = [&aLayoutNodes](const OUString& rName, double& rTop,
                                              double& rHeight) {
            for (const auto& rHolder : aLayoutNodes)
            {
                if (!rHolder.second)
                    continue;
                std::vector<Constraint> aOwn;
                gatherOwnConstraints(*rHolder.second, aOwn);
                bool bHeight(false);
                double fTop(0.0);
                double fHeight(0.0);
                for (const Constraint& rConstraint : aOwn)
                {
                    if (rConstraint.mnFor != XML_ch || rConstraint.msForName != rName
                        || rConstraint.mnRefType != XML_h || !rConstraint.msRefForName.isEmpty())
                        continue;
                    if (rConstraint.mnType == XML_t)
                        fTop = rConstraint.mfFactor;
                    else if (rConstraint.mnType == XML_h && rConstraint.mfFactor > 0.0)
                    {
                        fHeight = rConstraint.mfFactor;
                        bHeight = true;
                    }
                }
                if (bHeight)
                {
                    rTop = fTop;
                    rHeight = fHeight;
                    return true;
                }
            }
            return false;
        };
        double fSourceTop(0.0), fSourceHeight(1.0), fTargetTop(0.0), fTargetHeight(1.0);
        if (aPartsOf(pConn->getNamedParam(XML_srcNode), fSourceTop, fSourceHeight)
            && aPartsOf(pConn->getNamedParam(XML_dstNode), fTargetTop, fTargetHeight))
            fLevelGap = std::max(0.0, fLevelGap
                                          - (1.0 - fSourceTop - fSourceHeight) * fLevelHeight
                                          - fTargetTop * fLevelHeight);
        break;
    }

    for (UprightNode& rRoot : aRoots)
        shapeUpright(rRoot, fGap);
    std::vector<double> aOffsets;
    std::vector<std::pair<double, double>> aContour;
    packUpright(aRoots, fGap, aOffsets, aContour);
    double fLeft(std::numeric_limits<double>::max()), fRight(-std::numeric_limits<double>::max());
    size_t nDepth(0);
    for (const auto& rReach : aContour)
    {
        fLeft = std::min(fLeft, rReach.first);
        fRight = std::max(fRight, rReach.second);
    }
    for (const UprightNode& rRoot : aRoots)
        nDepth = std::max(nDepth, uprightDepth(rRoot));
    const double fWidth(fRight - fLeft);
    const double fHeight(nDepth * fLevelHeight + (nDepth - 1) * fLevelGap);
    if (fWidth <= 0.0 || fHeight <= 0.0)
        return false;
    const double fScale(
        std::min(rShape->getSize().Width / fWidth, rShape->getSize().Height / fHeight));

    // In the middle of the room across, and down as the node above says: a lin that stacks its
    // children from the top, vertAlign t, has the hierarchy at the top of the room it gave it, a
    // spacer above it and nothing below; otherwise it stands in the middle.
    sal_Int32 nVerticalAlign(XML_mid);
    {
        const OUString aAbove(presentationParentOf(rDgm, rShape->getDiagramDataModelID()));
        const rtl::Reference<svx::diagram::Point> xAbove(rDgm.getData()->getPointByModelID(aAbove));
        if (xAbove.is() && rDgm.getLayout()->getNode())
        {
            // the node above is not below this one, so the whole layout is searched for it
            std::map<OUString, const LayoutNode*> aWholeLayout;
            aWholeLayout[rDgm.getLayout()->getNode()->getName()]
                = rDgm.getLayout()->getNode().get();
            gatherLayoutNodes(*rDgm.getLayout()->getNode(), aWholeLayout);
            const auto aAboveNode
                = aWholeLayout.find(xAbove->getPresentation().msPresentationLayoutName);
            const LayoutNode* pAboveNode(aAboveNode != aWholeLayout.end() ? aAboveNode->second
                                                                          : nullptr);
            const AlgAtom* pAboveAlg(pAboveNode ? algorithmOf(rDgm, *pAboveNode, xAbove) : nullptr);
            if (pAboveAlg && pAboveAlg->getType() == XML_lin)
            {
                const auto aAlign = pAboveAlg->getMap().find(XML_vertAlign);
                if (aAlign != pAboveAlg->getMap().end())
                    nVerticalAlign = aAlign->second;
            }
        }
    }
    const double fLeftPad((rShape->getSize().Width / fScale - fWidth) / 2.0);
    const double fRoomBelow(rShape->getSize().Height / fScale - fHeight);
    const double fTopPad(nVerticalAlign == XML_t ? 0.0
                                                  : nVerticalAlign == XML_b ? fRoomBelow
                                                                            : fRoomBelow / 2.0);
    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    rMutable.getLaidOutSideways().insert(rShape.get());
    for (size_t nRoot = 0; nRoot < aRoots.size(); ++nRoot)
        placeUpright(rMutable, aRoots[nRoot], aOffsets[nRoot] - fLeft + fLeftPad, fTopPad, fScale,
                     fLevelGap, fLevelHeight);

    // a connector of the row itself has nothing to join, it is left where it is
    return true;
}

void AlgAtom::layoutShape(const SmartArtDiagram& rDgm, const ShapePtr& rShape, const std::vector<Constraint>& rConstraints,
                          const std::vector<Rule>& rRules)
{
    // A group with nothing in it is a space, and a line and a ring give a space the room it asks
    // for, so they keep theirs: on a ring a space takes a place between two nodes. A snake reads
    // the width a space states before it takes it out itself, that is its gap. The other
    // algorithms give a space no room of its own, so an empty group is taken out before they run.
    // A space a connector runs from or to, the dummy point of a process's node, stays and is laid
    // out where its constraints put it, so that the connector finds the place it is to join.
    if (mnType != XML_lin && mnType != XML_cycle && mnType != XML_snake)
    {
        std::set<OUString> aConnectorEnds;
        if (rDgm.getLayout() && rDgm.getLayout()->getNode())
            gatherConnectorEnds(*rDgm.getLayout()->getNode(), aConnectorEnds);
        std::erase_if(rShape->getChildren(), [&aConnectorEnds](const ShapePtr& aChild) {
            return aChild->getServiceName() == "com.sun.star.drawing.GroupShape"
                   && aChild->getChildren().empty()
                   && !aConnectorEnds.count(aChild->getInternalName());
        });
    }

    switch(mnType)
    {
        case XML_composite:
        {
            CompositeAlg::layoutShapeChildren(rDgm, *this, rShape, rConstraints);
            break;
        }

        case XML_conn:
        {
            if (rShape->getSubType() == XML_conn)
            {
                // There is no shape type "conn", replace it by an arrow based
                // on the direction of the parent linear layout.
                sal_Int32 nType = getConnectorType();

                rShape->setSubType(nType);
                rShape->getCustomShapeProperties()->setShapePresetType(nType);

                // the shaft of such an arrow is its stem over the head's height, stemThick and
                // hArH as the connector states them, 0.6 where nothing is stated; the head is
                // half the shorter side
                if (nType == XML_rightArrow || nType == XML_leftArrow
                    || nType == XML_leftRightArrow)
                {
                    double fStemOfWidth(0.0), fHeadOfWidth(1.0), fStemOfHead(0.0);
                    for (const Constraint& rOwn : collectDirectConstraints(getLayoutNode()))
                    {
                        if (!rOwn.msForName.isEmpty() || rOwn.mfFactor <= 0.0)
                            continue;
                        if (rOwn.mnType == XML_stemThick && rOwn.mnRefType == XML_w)
                            fStemOfWidth = rOwn.mfFactor;
                        else if (rOwn.mnType == XML_stemThick && rOwn.mnRefType == XML_hArH)
                            fStemOfHead = rOwn.mfFactor;
                        else if (rOwn.mnType == XML_hArH && rOwn.mnRefType == XML_w)
                            fHeadOfWidth = rOwn.mfFactor;
                    }
                    double fStem(0.6);
                    if (fStemOfHead > 0.0)
                        fStem = fStemOfHead;
                    else if (fStemOfWidth > 0.0 && fHeadOfWidth > 0.0)
                        fStem = fStemOfWidth / fHeadOfWidth;
                    fStem = std::clamp(fStem, 0.0, 1.0);
                    rShape->getCustomShapeProperties() = std::make_shared<CustomShapeProperties>(
                        *rShape->getCustomShapeProperties());
                    auto& rGuides(rShape->getCustomShapeProperties()->getAdjustmentGuideList());
                    setGuideValue(rGuides, u"adj1"_ustr,
                                  static_cast<sal_Int32>(std::lround(fStem * 100000.0)));
                    setGuideValue(rGuides, u"adj2"_ustr, 50000);
                }
            }

            // A spoke the ring has sized as a whole keeps its size.
            if (const_cast<SmartArtDiagram&>(rDgm).getLaidOutSideways().count(rShape.get()))
                break;

            // A connector that spans the ring it stands on, its diam being the diam of the
            // ring, is as large as the ring and keeps that. Its own proportions, an h stated as
            // a part of its w, describe a connector between two nodes and not this one.
            bool bSpansTheRing(false);
            for (const Constraint& rConstraint : rConstraints)
                if (XML_diam == rConstraint.mnType && XML_diam == rConstraint.mnRefType
                    && rConstraint.mfFactor > 0.0
                    && (rConstraint.msForName == rShape->getInternalName()
                        || (rConstraint.msForName.isEmpty()
                            && XML_sibTrans == rConstraint.mnPointType)))
                    bSpansTheRing = true;
            if (bSpansTheRing)
                break;

            // Parse constraints to adjust the size.
            const std::vector<Constraint> aDirectConstraints(
                collectDirectConstraints(getLayoutNode()));

            LayoutPropertyMap aProperties;
            LayoutProperty& rParent = aProperties[u""_ustr];
            rParent[XML_w] = rShape->getSize().Width;
            rParent[XML_h] = rShape->getSize().Height;
            rParent[XML_l] = 0;
            rParent[XML_t] = 0;
            rParent[XML_r] = rShape->getSize().Width;
            rParent[XML_b] = rShape->getSize().Height;
            for (const auto& rConstr : aDirectConstraints)
            {
                const LayoutPropertyMap::const_iterator aRef
                    = aProperties.find(rConstr.msRefForName);
                if (aRef != aProperties.end())
                {
                    const LayoutProperty::const_iterator aRefType
                        = aRef->second.find(rConstr.mnRefType);
                    if (aRefType != aRef->second.end())
                        aProperties[rConstr.msForName][rConstr.mnType]
                            = aRefType->second * rConstr.mfFactor;
                }
            }
            awt::Size aSize;
            aSize.Width = rParent[XML_w];
            aSize.Height = rParent[XML_h];
            // The h of a connector with a body is the reach of its head, and the band is 0.6 of
            // that, as the stem of a circular arrow is 0.6 of its h. A bar with no head is the
            // band alone: Vertical_Bending_Process states its bars as 0.25 of the node's
            // height, 1452 in units of 360 EMU, and draws them 871 thick.
            // A line, dim 1D, has no band and keeps its stated thickness, Basic_Radial's spokes.
            const auto aDimension = maMap.find(XML_dim);
            const bool bLine(aDimension != maMap.end() && aDimension->second == XML_1D);
            if (rShape->getSubType() == XML_rect && !bLine)
                aSize.Height = static_cast<sal_Int32>(aSize.Height * 0.6);
            // keep center position
            awt::Point aPos = rShape->getPosition();
            aPos.X += (rShape->getSize().Width - aSize.Width) / 2;
            aPos.Y += (rShape->getSize().Height - aSize.Height) / 2;

            rShape->setPosition(aPos);
            rShape->setSize(aSize);

            break;
        }

        case XML_cycle:
        {
            if (rShape->getChildren().empty())
                break;

            const sal_Int32 nStartAngle = maMap.count(XML_stAng) ? maMap.find(XML_stAng)->second : 0;
            const sal_Int32 nSpanAngle = maMap.count(XML_spanAng) ? maMap.find(XML_spanAng)->second : 360;
            const sal_Int32 nRotationPath = maMap.count(XML_rotPath) ? maMap.find(XML_rotPath)->second : XML_none;
            const sal_Int32 nctrShpMap = maMap.count(XML_ctrShpMap) ? maMap.find(XML_ctrShpMap)->second : XML_none;
            const awt::Size aCenter(rShape->getSize().Width / 2, rShape->getSize().Height / 2);

            // A child of the ring states its own proportions, the height as a part of the width,
            // and a square one says so with a factor of one. Where it states none the height
            // follows the parent, which is what every one of them used to do.
            std::map<OUString, double> aHeightOfWidth;
            gatherHeightOfWidthFactors(mrLayoutNode, aHeightOfWidth);
            // The ring may state the height for every node at once, "h for ch ptType node
            // refType w refFor ch refPtType node op equ" on Converging_Arrows, square arrows
            // pointing at the middle; that holds for each node that says nothing of its own.
            for (const Constraint& rConstraint : rConstraints)
            {
                if (rConstraint.mnType != XML_h || rConstraint.mnFor != XML_ch
                    || !rConstraint.msForName.isEmpty() || rConstraint.mnPointType != XML_node
                    || rConstraint.mnRefType != XML_w || rConstraint.mnRefFor != XML_ch
                    || !rConstraint.msRefForName.isEmpty() || rConstraint.mfFactor <= 0.0)
                    continue;
                for (const ShapePtr& pChild : rShape->getChildren())
                    if (pChild->getSubType() != XML_conn
                        && pChild->getDataNodeType() != XML_sibTrans
                        && pChild->getDataNodeType() != XML_parTrans
                        && !aHeightOfWidth.count(pChild->getInternalName()))
                        aHeightOfWidth[pChild->getInternalName()] = rConstraint.mfFactor;
            }

            // A line between the shapes states how thick it is, h with a val in millimetres.
            std::map<OUString, double> aStatedHeight;
            gatherStatedHeights(mrLayoutNode, aStatedHeight);

            // Where a spoke starts and ends, and how much of its way it leaves free at the start.
            std::map<OUString, std::pair<sal_Int32, sal_Int32>> aEndSites;
            gatherEndSites(mrLayoutNode, aEndSites);

            // A spoke that is a line, dim 1D, and states no thickness is a line of nothing:
            // Radial_Cluster's spokes have no height in the drawing, where we gave them the
            // twelfth of the box every unstated connector gets.
            std::set<OUString> aLinesOfNothing;
            {
                std::map<OUString, const LayoutNode*> aRingNodes;
                gatherLayoutNodes(mrLayoutNode, aRingNodes);
                for (const auto& rEntry : aRingNodes)
                    if (rEntry.second && !aStatedHeight.count(rEntry.first)
                        && drawsALine(*rEntry.second))
                        aLinesOfNothing.insert(rEntry.first);
            }
            std::map<OUString, double> aBeginPads;
            gatherBeginPads(mrLayoutNode, aBeginPads);
            // the connectors that state a begPad or an endPad at all, with nothing on it as well
            std::set<OUString> aPadsStated;
            {
                std::map<OUString, const LayoutNode*> aRingNodes;
                gatherLayoutNodes(mrLayoutNode, aRingNodes);
                for (const auto& rEntry : aRingNodes)
                {
                    if (!rEntry.second)
                        continue;
                    std::vector<Constraint> aOwn;
                    gatherOwnConstraints(*rEntry.second, aOwn);
                    for (const Constraint& rOwn : aOwn)
                        if (rOwn.mnType == XML_begPad || rOwn.mnType == XML_endPad)
                        {
                            aPadsStated.insert(rEntry.first);
                            break;
                        }
                }
            }

            // A layout can state the width of one child as a part of the width of another,
            // the node as 1.5 of the middle shape say, or 0.7 of it. Those parts are kept, by
            // name, against the width every child would have on its own, and a chain of them
            // is followed as far as it goes.
            std::map<OUString, double> aWidthOf;

            // A ring that states no spacing at all, sp or sibSp, and has a middle shape,
            // ctrShpMap fNode, takes the middle's parts of its own width and the names of the
            // layout's own as weights of its children, against the plain width, a quarter of
            // the ring's: Radial_Cluster's middle is 0.3 of the box, its nodes 0.67 of the
            // middle by "userS ... refType w refFor ch refForName singleCenter fact 0.67" and
            // "w refType userS", and as high as wide by "h ... refType w refFor ch singleCenter".
            // A ring with a spacing keeps the plain sizes and the fit that scales them, which is
            // what the drawings of those rings do.
            bool bStatesASpacing(false);
            for (const Constraint& rConstraint : rConstraints)
                if (rConstraint.mnType == XML_sp || rConstraint.mnType == XML_sibSp)
                    bStatesASpacing = true;
            const OUString aMiddleName(nctrShpMap == XML_fNode && !bStatesASpacing
                                               && !rShape->getChildren().empty()
                                           ? rShape->getChildren().front()->getInternalName()
                                           : OUString());
            if (!aMiddleName.isEmpty())
            {
                for (const Constraint& rConstraint : rConstraints)
                {
                    if (rConstraint.mnFor != XML_ch || rConstraint.msForName != aMiddleName
                        || rConstraint.mfFactor <= 0.0)
                        continue;
                    if (rConstraint.msRefForName == aMiddleName && rConstraint.mnType == XML_h
                        && rConstraint.mnRefType == XML_w && !aHeightOfWidth.count(aMiddleName))
                        aHeightOfWidth[aMiddleName] = rConstraint.mfFactor;
                    else if (rConstraint.msRefForName.isEmpty() && rConstraint.mnType == XML_w
                             && rConstraint.mnRefType == XML_w && !aWidthOf.count(aMiddleName))
                        aWidthOf[aMiddleName] = 4.0 * rConstraint.mfFactor;
                }
                std::map<sal_Int32, std::pair<OUString, double>> aNamedParts;
                for (const Constraint& rConstraint : rConstraints)
                    if (isUserVariable(rConstraint.mnType) && rConstraint.mnRefType == XML_w
                        && !rConstraint.msRefForName.isEmpty() && rConstraint.mfFactor > 0.0)
                        aNamedParts[rConstraint.mnType]
                            = { rConstraint.msRefForName, rConstraint.mfFactor };
                if (!aNamedParts.empty())
                {
                    std::map<OUString, const LayoutNode*> aRingNodes;
                    gatherLayoutNodes(mrLayoutNode, aRingNodes);
                    for (const auto& rEntry : aRingNodes)
                    {
                        if (!rEntry.second || aWidthOf.count(rEntry.first))
                            continue;
                        for (const Constraint& rOwn : collectDirectConstraints(*rEntry.second))
                        {
                            if (rOwn.mnType != XML_w || !isUserVariable(rOwn.mnRefType)
                                || !rOwn.msForName.isEmpty())
                                continue;
                            const auto aPart = aNamedParts.find(rOwn.mnRefType);
                            if (aPart == aNamedParts.end())
                                continue;
                            const auto aOf = aWidthOf.find(aPart->second.first);
                            const double fOf(aOf == aWidthOf.end() ? 1.0 : aOf->second);
                            aWidthOf[rEntry.first] = fOf * aPart->second.second
                                                     * (rOwn.mfFactor > 0.0 ? rOwn.mfFactor : 1.0);
                            break;
                        }
                    }
                }
            }
            // A ring with a middle shape and a spacing states the middle and the nodes as parts
            // of its own width, Radial_Venn's middle the whole and its nodes 0.5 of it, with an
            // sp of -0.2 of a node between: the spacing fit below scales them all alike, so
            // what counts is how they stand to each other, against the plain quarter.
            if (nctrShpMap == XML_fNode && bStatesASpacing)
            {
                std::map<OUString, double> aHeightPart;
                for (const Constraint& rConstraint : rConstraints)
                {
                    if (rConstraint.mnFor != XML_ch || rConstraint.msForName.isEmpty()
                        || !rConstraint.msRefForName.isEmpty() || rConstraint.mfFactor <= 0.0)
                        continue;
                    if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_w
                        && !aWidthOf.count(rConstraint.msForName))
                        aWidthOf[rConstraint.msForName] = 4.0 * rConstraint.mfFactor;
                    if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_h)
                        aHeightPart[rConstraint.msForName] = rConstraint.mfFactor;
                }
                // the height as a part of the ring's height, against the width as a part of
                // the ring's width, is a height of the width
                for (const auto& rPart : aHeightPart)
                {
                    const auto aWidth = aWidthOf.find(rPart.first);
                    if (aWidth == aWidthOf.end() || aHeightOfWidth.count(rPart.first)
                        || rShape->getSize().Width <= 0)
                        continue;
                    aHeightOfWidth[rPart.first] = rPart.second * rShape->getSize().Height
                                                  / (aWidth->second / 4.0
                                                     * rShape->getSize().Width);
                }
            }
            for (int nPass = 0; nPass < 4; ++nPass)
                for (const Constraint& rConstraint : rConstraints)
                {
                    if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
                        || rConstraint.msForName.isEmpty() || rConstraint.msRefForName.isEmpty()
                        || rConstraint.msForName == rConstraint.msRefForName
                        || rConstraint.mfFactor <= 0.0)
                        continue;
                    const auto aBase = aWidthOf.find(rConstraint.msRefForName);
                    aWidthOf[rConstraint.msForName]
                        = (aBase == aWidthOf.end() ? 1.0 : aBase->second) * rConstraint.mfFactor;
                }

            const auto aSizeOfChild = [&aHeightOfWidth, &aWidthOf](const OUString& rName,
                                                                   sal_Int32 nWidth,
                                                                   sal_Int32 nHeight) {
                const auto aPart = aWidthOf.find(rName);
                const sal_Int32 nOwnWidth(
                    aPart == aWidthOf.end() ? nWidth
                                            : static_cast<sal_Int32>(nWidth * aPart->second));
                const auto aFound = aHeightOfWidth.find(rName);
                return aFound == aHeightOfWidth.end()
                           ? awt::Size(nOwnWidth, nHeight)
                           : awt::Size(nOwnWidth,
                                       static_cast<sal_Int32>(nOwnWidth * aFound->second));
            };

            const awt::Size aConnectorSize(rShape->getSize().Width / 12, rShape->getSize().Height / 12);

            // A layout can say that the shapes between the nodes go on a ring of their own,
            // "diam" being the width of the ring the children are placed on. When that ring is
            // the one of the parent the shape spans the whole circle, it is drawn in the middle
            // and it takes no place of its own between two nodes.
            std::set<OUString> aSpanningNames;
            bool bSpanningTransitions(false);

            for (const Constraint& rConstraint : rConstraints)
            {
                if (XML_diam != rConstraint.mnType || XML_diam != rConstraint.mnRefType
                    || rConstraint.mfFactor <= 0.0)
                    continue;

                if (!rConstraint.msForName.isEmpty())
                    aSpanningNames.insert(rConstraint.msForName);
                else if (XML_sibTrans == rConstraint.mnPointType)
                    bSpanningTransitions = true;
            }

            // The ring may hand its diam to its transitions as a name of the layout's own,
            // "userA for ch ptType sibTrans refType diam" on Continuous_Cycle, and a transition
            // whose own layout node states its diam as a part of that name, "diam refType userA
            // fact 1.26" for four nodes, is a circle round the ring and no connector between
            // two nodes. Its box is that part of the ring's diameter, the circle through the
            // middles of the nodes, with the stem the ring states for it, "stemThick ...
            // refType diam fact 0.065", on either side: 1.39 of the ring, 5177256 in the
            // drawing for a ring of 3717957, read off that one file.
            //
            // Where the ring states the head of that arrow as well, "wArH ... fact 0.05" and
            // "hArH ... fact 0.1" as parts of its diam, the transition is a block arrow bent
            // round the ring, drawn as one of the circular arrow presets, and its box, its
            // place and its adjustments follow from what is stated. That is read off the
            // Continuous_Cycle files with three to ten nodes, and the numbers are given
            // where they are put to use below.
            struct RingArrow
            {
                // the arrow's circle as a part of the ring's diameter, the diam the
                // transition states through the name the ring handed it
                double mfCirclePart = 0.0;
                // stemThick, wArH and hArH, parts of the ring's diameter: how thick the band
                // is, how long the head is along the arc and how far it reaches across
                double mfStem = 0.0;
                double mfHeadLength = 0.0;
                double mfHeadReach = 0.0;
                // begPad and endPad, parts of the way between two nodes, which is the width
                // of a node
                double mfBeginPad = 0.0;
                double mfEndPad = 0.0;
                // a negative diam runs the arrow clockwise, the norm way round
                bool mbClockwise = true;
            };
            std::map<const Shape*, RingArrow> aSpanOfRing;
            {
                std::map<sal_Int32, double> aRingDiamNames;
                RingArrow aOfRing;
                for (const Constraint& rConstraint : rConstraints)
                {
                    if (rConstraint.mnRefType != XML_diam || rConstraint.mnFor != XML_ch
                        || (rConstraint.mnPointType != XML_sibTrans
                            && rConstraint.msForName.isEmpty()))
                        continue;
                    if (isUserVariable(rConstraint.mnType))
                        aRingDiamNames[rConstraint.mnType] = rConstraint.mfFactor;
                    else if (rConstraint.mnType == XML_stemThick && rConstraint.mfFactor > 0.0)
                        aOfRing.mfStem = rConstraint.mfFactor;
                    else if (rConstraint.mnType == XML_wArH && rConstraint.mfFactor > 0.0)
                        aOfRing.mfHeadLength = rConstraint.mfFactor;
                    else if (rConstraint.mnType == XML_hArH && rConstraint.mfFactor > 0.0)
                        aOfRing.mfHeadReach = rConstraint.mfFactor;
                }
                if (!aRingDiamNames.empty())
                {
                    std::map<OUString, const LayoutNode*> aRingNodes;
                    gatherLayoutNodes(mrLayoutNode, aRingNodes);
                    for (const ShapePtr& pChild : rShape->getChildren())
                    {
                        const auto aNode = aRingNodes.find(pChild->getInternalName());
                        if (aNode == aRingNodes.end() || !aNode->second)
                            continue;
                        rtl::Reference<svx::diagram::Point> xChildPoint;
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                            if (rEntry.second == pChild)
                            {
                                xChildPoint = rEntry.first;
                                break;
                            }
                        std::vector<Constraint> aOwn;
                        gatherDecidedConstraints(rDgm, *aNode->second, xChildPoint, aOwn);
                        std::optional<RingArrow> aArrow;
                        for (const Constraint& rOwn : aOwn)
                        {
                            const auto aName = aRingDiamNames.find(rOwn.mnRefType);
                            if (rOwn.mnType == XML_diam && rOwn.msForName.isEmpty()
                                && aName != aRingDiamNames.end() && rOwn.mfFactor != 0.0)
                            {
                                aArrow = aOfRing;
                                aArrow->mfCirclePart
                                    = std::abs(rOwn.mfFactor * aName->second);
                                aArrow->mbClockwise = rOwn.mfFactor * aName->second < 0.0;
                                break;
                            }
                        }
                        if (!aArrow)
                            continue;
                        // the transition's own words on the head, the stem and the pads
                        for (const Constraint& rOwn : aOwn)
                        {
                            if (!rOwn.msForName.isEmpty())
                                continue;
                            const auto aName = aRingDiamNames.find(rOwn.mnRefType);
                            if (aName != aRingDiamNames.end() && rOwn.mfFactor > 0.0)
                            {
                                const double fPart(rOwn.mfFactor * std::abs(aName->second));
                                if (rOwn.mnType == XML_stemThick)
                                    aArrow->mfStem = fPart;
                                else if (rOwn.mnType == XML_wArH)
                                    aArrow->mfHeadLength = fPart;
                                else if (rOwn.mnType == XML_hArH)
                                    aArrow->mfHeadReach = fPart;
                            }
                            else if (rOwn.mnRefType == XML_connDist)
                            {
                                if (rOwn.mnType == XML_begPad)
                                    aArrow->mfBeginPad = rOwn.mfFactor;
                                else if (rOwn.mnType == XML_endPad)
                                    aArrow->mfEndPad = rOwn.mfFactor;
                            }
                        }
                        aSpanOfRing[pChild.get()] = *aArrow;
                    }
                }
            }

            const auto aSpansTheCircle = [&](const oox::drawingml::ShapePtr& rCycleChild) {
                return aSpanningNames.count(rCycleChild->getInternalName()) > 0
                       || (bSpanningTransitions && rCycleChild->getSubType() == XML_conn)
                       || aSpanOfRing.count(rCycleChild.get()) > 0;
            };

            std::vector<oox::drawingml::ShapePtr> aCycleChildren = rShape->getChildren();

            // With the first node in the middle, a parTrans is the line from that node out to
            // the one it joins to the ring, a spoke. It takes no place on the ring.
            const auto aIsSpoke = [nctrShpMap](const oox::drawingml::ShapePtr& rCycleChild) {
                return nctrShpMap == XML_fNode && rCycleChild->getSubType() == XML_conn
                       && rCycleChild->getDataNodeType() == XML_parTrans;
            };

            // Only the shapes that take a place around the circle share it out between them.
            sal_Int32 nPlacesOnTheCircle(0);

            for (const auto& rCycleChild : aCycleChildren)
                if (!aSpansTheCircle(rCycleChild) && !aIsSpoke(rCycleChild))
                    nPlacesOnTheCircle++;

            if (nctrShpMap == XML_fNode && nPlacesOnTheCircle > 0)
                nPlacesOnTheCircle--;

            sal_Int32 nPlainWidth(rShape->getSize().Width / 4);
            sal_Int32 nPlainHeight(rShape->getSize().Height / 4);

            // The ring has to hold the widest and the tallest of them, so those settle its size.
            // A node that states its height as a part of its width, Continuous_Cycle's nodes
            // half as high as wide, is that high and no higher, so the ring reaches further
            // down the room: its nodes fill the room's height in every one of the count
            // examples, and the plain height is only for a ring with no node on it.
            const auto aLargestChild = [&]() {
                sal_Int32 nRingChildWidth(0);
                sal_Int32 nRingChildHeight(0);
                for (const auto& rCycleChild : rShape->getChildren())
                    if (rCycleChild->getSubType() != XML_conn)
                    {
                        const awt::Size aOwn(aSizeOfChild(rCycleChild->getInternalName(),
                                                          nPlainWidth, nPlainHeight));
                        nRingChildWidth = std::max(nRingChildWidth, aOwn.Width);
                        nRingChildHeight = std::max(nRingChildHeight, aOwn.Height);
                    }
                if (nRingChildWidth <= 0 || nRingChildHeight <= 0)
                    return awt::Size(nPlainWidth, nPlainHeight);
                return awt::Size(nRingChildWidth, nRingChildHeight);
            };
            awt::Size aChildSize(aLargestChild());

            // The ring is as large as the room allows once the nodes on it are in the room too.
            // Where the nodes stand depends on their angles: three of them starting at the top
            // reach the top of the ring and only half way down the other side, so the ring can
            // be larger than the room halved, and it is not in the middle of the room then but
            // where the nodes together are. Their reach either way is worked out from the angles
            // and the ring is fitted to that, and the middle of the ring is put where the nodes
            // as a whole sit in the middle of the room.
            // With the first node in the middle, the middle is part of what has to fit, so
            // the reach starts out holding it; a ring of nodes alone starts out empty.
            const bool bMiddleCounts(nctrShpMap == XML_fNode);
            double fLeast(bMiddleCounts ? 0.0 : 1.0), fMost(bMiddleCounts ? 0.0 : -1.0);
            double fUp(bMiddleCounts ? 0.0 : 1.0), fDown(bMiddleCounts ? 0.0 : -1.0);
            const sal_Int32 nPlaces(std::max<sal_Int32>(nPlacesOnTheCircle, 1));
            // Round the whole ring the places share the turn between them. On a part of the
            // ring the first place is at the start angle and the last one at the end of the
            // span, so the turn between two places is the span shared between the gaps.
            const bool bWholeRing(std::abs(nSpanAngle) % 360 == 0);
            const double fStep(bWholeRing || nPlaces < 2
                                   ? static_cast<double>(nSpanAngle) / nPlaces
                                   : static_cast<double>(nSpanAngle) / (nPlaces - 1));
            for (sal_Int32 nPlace = 0; nPlace < nPlaces; ++nPlace)
            {
                const double fAt(basegfx::deg2rad(nPlace * fStep + nStartAngle));
                fLeast = std::min(fLeast, sin(fAt));
                fMost = std::max(fMost, sin(fAt));
                fUp = std::min(fUp, -cos(fAt));
                fDown = std::max(fDown, -cos(fAt));
            }

            const double fReachX(fMost - fLeast);
            const double fReachY(fDown - fUp);
            const double fRoomX(std::max<double>(rShape->getSize().Width - aChildSize.Width, 1.0));
            const double fRoomY(
                std::max<double>(rShape->getSize().Height - aChildSize.Height, 1.0));
            double fRadius(std::min(fRoomX, fRoomY) / 2.0);
            if (fReachX > 0.01 && fReachY > 0.01)
                fRadius = std::min(fRoomX / fReachX, fRoomY / fReachY);
            else if (fReachX > 0.01)
                fRadius = fRoomX / fReachX;
            else if (fReachY > 0.01)
                fRadius = fRoomY / fReachY;

            // with a node in the middle and sp stated, the ring is not fitted to the room: its
            // radius is the middle shape's edge, sp and the node's edge along the spoke, and the
            // shapes are scaled until the whole fits. sp scales along, so this takes a few rounds.
            // The edge of a shape along a spoke: an ellipse reaches its half width whichever way, a
            // box the side the spoke runs into, at most its corner
            const auto aEdgeAlong = [](const oox::drawingml::ShapePtr& rChild,
                                       const awt::Size& rSize, double fAt) {
                if (rChild->getCustomShapeProperties()->getShapePresetType() == XML_ellipse)
                    return std::min(rSize.Width, rSize.Height) / 2.0;
                const double fSin(std::abs(sin(fAt)));
                const double fCos(std::abs(cos(fAt)));
                double fEdge(std::hypot(rSize.Width, rSize.Height) / 2.0);
                if (fSin > 0.01)
                    fEdge = std::min(fEdge, rSize.Width / 2.0 / fSin);
                if (fCos > 0.01)
                    fEdge = std::min(fEdge, rSize.Height / 2.0 / fCos);
                return fEdge;
            };

            std::optional<awt::Size> aMiddleAt;
            if (bMiddleCounts && aCycleChildren.size() > 1)
            {
                double fSpacePart(0.0);
                double fSpaceFixed(0.0);
                OUString aSpaceOf;
                for (const Constraint& rConstraint : rConstraints)
                {
                    if (rConstraint.mnType != XML_sp || !rConstraint.msForName.isEmpty())
                        continue;
                    // a part of less than nothing is an overlap of the middle and the nodes
                    if (rConstraint.mnRefType == XML_w && !rConstraint.msRefForName.isEmpty()
                        && rConstraint.mfFactor != 0.0)
                    {
                        fSpacePart = rConstraint.mfFactor;
                        aSpaceOf = rConstraint.msRefForName;
                    }
                    else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 0.0)
                        fSpaceFixed = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm,
                                                    o3tl::Length::emu);
                }

                if (fSpacePart != 0.0 || fSpaceFixed > 0.0)
                {
                    const oox::drawingml::ShapePtr& rMiddle(aCycleChildren.front());

                    // The ring for a scale of the plain sizes, and the box the middle shape and
                    // the nodes on that ring cover, against the middle of the ring.
                    struct RingAndCover
                    {
                        double fRing;
                        double fLeft, fTop, fRight, fBottom;
                    };
                    // the nodes keep the sibSp between them too, from where the line between two
                    // neighbours leaves one to where it enters the other, and the ring is the
                    // larger of that and the spokes
                    double fSiblingPart(0.0);
                    double fSiblingFixed(0.0);
                    OUString aSiblingOf;
                    for (const Constraint& rConstraint : rConstraints)
                    {
                        if (rConstraint.mnType != XML_sibSp || !rConstraint.msForName.isEmpty())
                            continue;
                        if (rConstraint.mnRefType == XML_w && !rConstraint.msRefForName.isEmpty()
                            && rConstraint.mfFactor != 0.0)
                        {
                            fSiblingPart = rConstraint.mfFactor;
                            aSiblingOf = rConstraint.msRefForName;
                        }
                        else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 0.0)
                            fSiblingFixed = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm,
                                                          o3tl::Length::emu);
                    }
                    // what a composite on the ring draws across, as parts of its width from its
                    // left: its children's l and w for the branch of its point; 0 and 1 where
                    // nothing is stated
                    std::map<OUString, const LayoutNode*> aRingNodes;
                    gatherLayoutNodes(getLayoutNode(), aRingNodes);
                    const auto aDrawnAcross = [&](const oox::drawingml::ShapePtr& rChild) {
                        std::pair<double, double> aAcross(0.0, 1.0);
                        const auto aNode = aRingNodes.find(rChild->getInternalName());
                        if (aNode == aRingNodes.end() || !aNode->second)
                            return aAcross;
                        rtl::Reference<svx::diagram::Point> xPoint;
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                            if (rEntry.second == rChild)
                            {
                                xPoint = rEntry.first;
                                break;
                            }
                        std::vector<Constraint> aOwn;
                        gatherDecidedConstraints(rDgm, *aNode->second, xPoint, aOwn);
                        std::map<OUString, double> aWidthPart, aLeftPart;
                        for (int nPass = 0; nPass < 3; ++nPass)
                            for (const Constraint& rOne : aOwn)
                            {
                                if (rOne.mnFor != XML_ch || rOne.msForName.isEmpty()
                                    || rOne.mnRefType != XML_w)
                                    continue;
                                const double fPart(rOne.mfFactor != 0.0 ? rOne.mfFactor : 1.0);
                                std::optional<double> oOf;
                                if (rOne.msRefForName.isEmpty())
                                    oOf = 1.0;
                                else if (aWidthPart.count(rOne.msRefForName))
                                    oOf = aWidthPart.at(rOne.msRefForName);
                                if (!oOf)
                                    continue;
                                if (rOne.mnType == XML_w)
                                    aWidthPart[rOne.msForName] = fPart * *oOf;
                                else if (rOne.mnType == XML_l)
                                    aLeftPart[rOne.msForName] = fPart * *oOf;
                            }
                        if (aWidthPart.empty())
                            return aAcross;
                        double fLeft(0.0), fRight(0.0);
                        for (const auto& [rName, fWidth] : aWidthPart)
                        {
                            const double fAt(aLeftPart.count(rName) ? aLeftPart.at(rName)
                                                                    : 0.0);
                            fLeft = std::min(fLeft, fAt);
                            fRight = std::max(fRight, fAt + fWidth);
                        }
                        aAcross = std::make_pair(fLeft, std::max(1.0, fRight));
                        return aAcross;
                    };
                    const auto aRingFor = [&](double fScale) {
                        const sal_Int32 nWidth(nPlainWidth * fScale);
                        const sal_Int32 nHeight(nPlainHeight * fScale);
                        const awt::Size aMiddle(
                            aSizeOfChild(rMiddle->getInternalName(), nWidth, nHeight));
                        double fSpace(fSpaceFixed * fScale);
                        if (fSpacePart != 0.0)
                            fSpace += fSpacePart * aSizeOfChild(aSpaceOf, nWidth, nHeight).Width;

                        RingAndCover aOut{ 0.0, -aMiddle.Width / 2.0, -aMiddle.Height / 2.0,
                                           aMiddle.Width / 2.0, aMiddle.Height / 2.0 };
                        sal_Int32 nPlace(0);
                        for (size_t nChild = 1; nChild < aCycleChildren.size(); ++nChild)
                        {
                            const oox::drawingml::ShapePtr& rChild(aCycleChildren[nChild]);
                            if (aSpansTheCircle(rChild) || aIsSpoke(rChild)
                                || rChild->getSubType() == XML_conn)
                                continue;
                            const double fAt(basegfx::deg2rad(nPlace * fStep + nStartAngle));
                            const awt::Size aOwn(
                                aSizeOfChild(rChild->getInternalName(), nWidth, nHeight));
                            aOut.fRing = std::max(aOut.fRing, aEdgeAlong(rMiddle, aMiddle, fAt)
                                                                  + fSpace
                                                                  + aEdgeAlong(rChild, aOwn, fAt));
                            ++nPlace;
                        }
                        if (fSiblingPart != 0.0 || fSiblingFixed > 0.0)
                        {
                            double fSibling(fSiblingFixed * fScale);
                            if (fSiblingPart != 0.0)
                                fSibling += fSiblingPart
                                            * aSizeOfChild(aSiblingOf, nWidth, nHeight).Width;
                            struct Placed
                            {
                                double fAt;
                                awt::Size aSize;
                                oox::drawingml::ShapePtr pShape;
                            };
                            std::vector<Placed> aPlaced;
                            for (size_t nChild = 1; nChild < aCycleChildren.size(); ++nChild)
                            {
                                const oox::drawingml::ShapePtr& rChild(aCycleChildren[nChild]);
                                if (aSpansTheCircle(rChild) || aIsSpoke(rChild)
                                    || rChild->getSubType() == XML_conn)
                                    continue;
                                aPlaced.push_back(
                                    { basegfx::deg2rad(aPlaced.size() * fStep + nStartAngle),
                                      aSizeOfChild(rChild->getInternalName(), nWidth, nHeight),
                                      rChild });
                            }
                            const size_t nPairs(bWholeRing ? aPlaced.size()
                                                           : aPlaced.size() - 1);
                            const double fHalfStep(basegfx::deg2rad(std::abs(fStep)) / 2.0);
                            for (size_t nPair = 0; aPlaced.size() >= 2 && nPair < nPairs
                                                   && sin(fHalfStep) > 0.001;
                                 ++nPair)
                            {
                                const auto& rFrom(aPlaced[nPair]);
                                const auto& rTo(aPlaced[(nPair + 1) % aPlaced.size()]);
                                // the line between the two, as an angle from the top like a spoke
                                const double fAlong(std::atan2(sin(rTo.fAt) - sin(rFrom.fAt),
                                                               cos(rTo.fAt) - cos(rFrom.fAt)));
                                aOut.fRing = std::max(
                                    aOut.fRing,
                                    (aEdgeAlong(rFrom.pShape, rFrom.aSize, fAlong)
                                     + aEdgeAlong(rTo.pShape, rTo.aSize, fAlong) + fSibling)
                                        / (2.0 * sin(fHalfStep)));
                            }
                        }
                        nPlace = 0;
                        for (size_t nChild = 1; nChild < aCycleChildren.size(); ++nChild)
                        {
                            const oox::drawingml::ShapePtr& rChild(aCycleChildren[nChild]);
                            if (aSpansTheCircle(rChild) || aIsSpoke(rChild)
                                || rChild->getSubType() == XML_conn)
                                continue;
                            const double fAt(basegfx::deg2rad(nPlace * fStep + nStartAngle));
                            const awt::Size aOwn(
                                aSizeOfChild(rChild->getInternalName(), nWidth, nHeight));
                            const double fX(aOut.fRing * sin(fAt));
                            const double fY(-aOut.fRing * cos(fAt));
                            const std::pair<double, double> aAcross(aDrawnAcross(rChild));
                            aOut.fLeft = std::min(aOut.fLeft, fX - aOwn.Width / 2.0
                                                                  + aAcross.first * aOwn.Width);
                            aOut.fRight = std::max(aOut.fRight, fX - aOwn.Width / 2.0
                                                                    + aAcross.second * aOwn.Width);
                            aOut.fTop = std::min(aOut.fTop, fY - aOwn.Height / 2.0);
                            aOut.fBottom = std::max(aOut.fBottom, fY + aOwn.Height / 2.0);
                            ++nPlace;
                        }
                        // a connector round the whole ring, as thick as a part of a node, counts
                        // as a circle of the ring and that thickness round the middle
                        for (const oox::drawingml::ShapePtr& rChild : aCycleChildren)
                        {
                            if (!aSpansTheCircle(rChild) || rChild->getSubType() != XML_conn)
                                continue;
                            for (const Constraint& rConstraint : rConstraints)
                            {
                                if (rConstraint.mnType != XML_h || rConstraint.mnRefType != XML_w
                                    || rConstraint.msForName != rChild->getInternalName()
                                    || rConstraint.msRefForName.isEmpty()
                                    || rConstraint.mfFactor <= 0.0)
                                    continue;
                                const double fReach(
                                    aOut.fRing
                                    + rConstraint.mfFactor
                                          * aSizeOfChild(rConstraint.msRefForName, nWidth, nHeight)
                                                .Width);
                                aOut.fLeft = std::min(aOut.fLeft, -fReach);
                                aOut.fRight = std::max(aOut.fRight, fReach);
                                aOut.fTop = std::min(aOut.fTop, -fReach);
                                aOut.fBottom = std::max(aOut.fBottom, fReach);
                            }
                        }
                        return aOut;
                    };

                    double fScale(1.0);
                    RingAndCover aRing(aRingFor(fScale));

                    // A rule may let a child's width go down to a part of what it refers to,
                    // "rule w for ch forName node fact 1" of Diverging_Radial, whose nodes are
                    // stated 1.25 of the middle: where the ring does not stand in the room at
                    // the stated sizes the width goes that far down before the whole is scaled,
                    // and the drawing has the nodes as large as the middle.
                    {
                        const double fWide(std::max(1.0, aRing.fRight - aRing.fLeft));
                        const double fHigh(std::max(1.0, aRing.fBottom - aRing.fTop));
                        const bool bOverflows(std::min(rShape->getSize().Width / fWide,
                                                       rShape->getSize().Height / fHigh)
                                              < 0.999);
                        bool bChanged(false);
                        for (const Rule& rRule : rRules)
                        {
                            if (!bOverflows || rRule.mnType != XML_w || rRule.msForName.isEmpty()
                                || !std::isfinite(rRule.mfFactor) || rRule.mfFactor <= 0.0)
                                continue;
                            for (const Constraint& rConstraint : rConstraints)
                            {
                                if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
                                    || rConstraint.msForName != rRule.msForName
                                    || rConstraint.msRefForName.isEmpty()
                                    || rConstraint.mfFactor <= rRule.mfFactor)
                                    continue;
                                const auto aBase = aWidthOf.find(rConstraint.msRefForName);
                                aWidthOf[rRule.msForName]
                                    = (aBase == aWidthOf.end() ? 1.0 : aBase->second)
                                      * rRule.mfFactor;
                                bChanged = true;
                                break;
                            }
                        }
                        if (bChanged)
                            aRing = aRingFor(fScale);
                    }
                    for (int nRound = 0; nRound < 8; ++nRound)
                    {
                        const double fWide(std::max(1.0, aRing.fRight - aRing.fLeft));
                        const double fHigh(std::max(1.0, aRing.fBottom - aRing.fTop));
                        const double fFit(std::min(rShape->getSize().Width / fWide,
                                                   rShape->getSize().Height / fHigh));
                        if (std::abs(fFit - 1.0) < 0.001)
                            break;
                        fScale *= fFit;
                        aRing = aRingFor(fScale);
                    }

                    if (aRing.fRing > 1.0)
                    {
                        nPlainWidth = static_cast<sal_Int32>(nPlainWidth * fScale);
                        nPlainHeight = static_cast<sal_Int32>(nPlainHeight * fScale);
                        aChildSize = aLargestChild();
                        fRadius = aRing.fRing;
                        aMiddleAt = awt::Size(
                            static_cast<sal_Int32>(aCenter.Width
                                                   - (aRing.fLeft + aRing.fRight) / 2.0),
                            static_cast<sal_Int32>(aCenter.Height
                                                   - (aRing.fTop + aRing.fBottom) / 2.0));
                    }
                }
                else if (!aMiddleName.isEmpty())
                {
                    // No spacing stated: the sizes stand as the constraints say, and the ring
                    // grows until what it covers, the middle shape and the nodes around it,
                    // fills the box one way or the other. Radial_Cluster's three nodes of 0.201
                    // of the box stand on a ring that reaches the box's width, 2497667 across.
                    const oox::drawingml::ShapePtr& rMiddle(aCycleChildren.front());
                    struct Cover
                    {
                        double fLeft, fTop, fRight, fBottom;
                    };
                    const auto aCoverFor = [&](double fRing) {
                        const awt::Size aMiddle(
                            aSizeOfChild(rMiddle->getInternalName(), nPlainWidth, nPlainHeight));
                        Cover aOut{ -aMiddle.Width / 2.0, -aMiddle.Height / 2.0,
                                    aMiddle.Width / 2.0, aMiddle.Height / 2.0 };
                        sal_Int32 nPlace(0);
                        for (size_t nChild = 1; nChild < aCycleChildren.size(); ++nChild)
                        {
                            const oox::drawingml::ShapePtr& rChild(aCycleChildren[nChild]);
                            if (aSpansTheCircle(rChild) || aIsSpoke(rChild)
                                || rChild->getSubType() == XML_conn)
                                continue;
                            const double fAt(basegfx::deg2rad(nPlace * fStep + nStartAngle));
                            const awt::Size aOwn(aSizeOfChild(rChild->getInternalName(),
                                                              nPlainWidth, nPlainHeight));
                            const double fX(fRing * sin(fAt));
                            const double fY(-fRing * cos(fAt));
                            aOut.fLeft = std::min(aOut.fLeft, fX - aOwn.Width / 2.0);
                            aOut.fRight = std::max(aOut.fRight, fX + aOwn.Width / 2.0);
                            aOut.fTop = std::min(aOut.fTop, fY - aOwn.Height / 2.0);
                            aOut.fBottom = std::max(aOut.fBottom, fY + aOwn.Height / 2.0);
                            ++nPlace;
                        }
                        return aOut;
                    };
                    const auto aFits = [&](const Cover& rCover) {
                        return rCover.fRight - rCover.fLeft <= rShape->getSize().Width
                               && rCover.fBottom - rCover.fTop <= rShape->getSize().Height;
                    };
                    // the ring the nodes touch the middle at is the least it can be
                    const auto aTouchRing = [&]() {
                        const awt::Size aMiddle(
                            aSizeOfChild(rMiddle->getInternalName(), nPlainWidth, nPlainHeight));
                        double fTouch(0.0);
                        sal_Int32 nPlace(0);
                        for (size_t nChild = 1; nChild < aCycleChildren.size(); ++nChild)
                        {
                            const oox::drawingml::ShapePtr& rChild(aCycleChildren[nChild]);
                            if (aSpansTheCircle(rChild) || aIsSpoke(rChild)
                                || rChild->getSubType() == XML_conn)
                                continue;
                            const double fAt(basegfx::deg2rad(nPlace * fStep + nStartAngle));
                            const awt::Size aOwn(aSizeOfChild(rChild->getInternalName(),
                                                              nPlainWidth, nPlainHeight));
                            fTouch = std::max(fTouch, aEdgeAlong(rMiddle, aMiddle, fAt)
                                                          + aEdgeAlong(rChild, aOwn, fAt));
                            ++nPlace;
                        }
                        return std::make_pair(fTouch, nPlace);
                    };
                    // Sizes the box cannot hold even with the nodes touching the middle shrink
                    // first, all by one factor, the way the fit above does with a spacing.
                    for (int nRound = 0; nRound < 8; ++nRound)
                    {
                        const Cover aCover(aCoverFor(aTouchRing().first));
                        const double fWide(std::max(1.0, aCover.fRight - aCover.fLeft));
                        const double fHigh(std::max(1.0, aCover.fBottom - aCover.fTop));
                        const double fFit(std::min(rShape->getSize().Width / fWide,
                                                   rShape->getSize().Height / fHigh));
                        if (fFit >= 0.999)
                            break;
                        nPlainWidth = static_cast<sal_Int32>(nPlainWidth * fFit);
                        nPlainHeight = static_cast<sal_Int32>(nPlainHeight * fFit);
                        aChildSize = aLargestChild();
                    }
                    const auto [fTouch, nPlace] = aTouchRing();
                    if (nPlace > 0 && fTouch > 1.0 && aFits(aCoverFor(fTouch)))
                    {
                        double fLow(fTouch);
                        double fHigh(fTouch + rShape->getSize().Width + rShape->getSize().Height);
                        for (int nRound = 0; nRound < 40; ++nRound)
                        {
                            const double fMiddle((fLow + fHigh) / 2.0);
                            if (aFits(aCoverFor(fMiddle)))
                                fLow = fMiddle;
                            else
                                fHigh = fMiddle;
                        }
                        const Cover aCover(aCoverFor(fLow));
                        fRadius = fLow;
                        aMiddleAt = awt::Size(
                            static_cast<sal_Int32>(aCenter.Width
                                                   - (aCover.fLeft + aCover.fRight) / 2.0),
                            static_cast<sal_Int32>(aCenter.Height
                                                   - (aCover.fTop + aCover.fBottom) / 2.0));
                    }
                }
            }

            // A ring of ellipses alone with sibSp stated as a part of the node's width: two
            // ellipses that follow each other stand a chord of the node's width and the sibSp
            // apart, an ellipse reaching half its width whichever way, and the node is as large
            // as the box allows with that ring around it. Basic_Cycle's four ellipses with a
            // sibSp of 0.5 stand 1.5 widths apart, the ring 1.0607 widths, and the whole as high
            // as the box, the node 1734343 for a box of 5418667. Only where the places are the
            // nodes and their connectors; a spacer among them is another matter.
            //
            // A box for a node stands the sibSp from the next one along the line between their
            // middles, from where that line leaves the one box to where it enters the other,
            // which for an ellipse is half its width and for a box the nearer of its edges along
            // the line. Multidirectional_Cycle's four boxes, h 0.5 of w, stand 1.3595 widths
            // apart for a sibSp of 0.65: the line leaves each box at 0.3536 of its width. A ring
            // that one of its children spans, Nondirectional_Cycle's arcs, is another matter
            // still, its boxes stand 1.5186 widths apart for a sibSp of 0.15.
            //
            // A sibSp stated as a bare value is a length in millimetres, and the node it stands
            // between is as wide as the ring's own width says, "w for ch ptType node refType w",
            // before anything is scaled: Continuous_Cycle's "sibSp val 15" is 15 mm of the
            // 8640000 the ring has, 0.0625 of the node, and the ring is what the widest pair of
            // boxes asks for along their chord, the horizontal pair with five nodes, the
            // diagonal ones with four, then the whole is scaled to fill the height. That gives
            // 4050000 for three nodes and 3623906 for four, the drawing's numbers, and holds
            // whether or not a transition spans the ring.
            // the sibSp, a part of the node's width, that a ring of boxes stands apart by
            double fBoxRingSibling(0.0);
            if (!bMiddleCounts && nPlacesOnTheCircle >= 2)
            {
                double fSiblingPart(0.0);
                for (const Constraint& rConstraint : rConstraints)
                {
                    if (rConstraint.mnType != XML_sibSp)
                        continue;
                    // a sibSp of less than nothing is an overlap along the chord, Converging_
                    // Arrows' four arrows of -0.35 pointing at the middle
                    if (rConstraint.mnRefType == XML_w && rConstraint.mnRefFor == XML_ch
                        && (rConstraint.mnRefPointType == XML_node
                            || !rConstraint.msRefForName.isEmpty())
                        && rConstraint.mfFactor != 0.0 && rConstraint.mnOperator != XML_lte
                        && rConstraint.mnOperator != XML_gte)
                        fSiblingPart = rConstraint.mfFactor;
                    // a sibSp of the cycle's own width is that part of a node as wide as the
                    // cycle
                    else if (rConstraint.mnRefType == XML_w && rConstraint.mnRefFor != XML_ch
                             && rConstraint.msRefForName.isEmpty() && rConstraint.mfFactor != 0.0
                             && rConstraint.mnOperator != XML_lte
                             && rConstraint.mnOperator != XML_gte)
                        fSiblingPart = rConstraint.mfFactor;
                    else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 0.0
                             && rShape->getSize().Width > 0)
                    {
                        fSiblingPart = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm,
                                                     o3tl::Length::emu)
                                       / rShape->getSize().Width;
                    }
                }
                // The places on the ring in their order, the nodes and the spacers among
                // them; a spacer stated as a part of the sibSp, Nondirectional_Cycle's "w for
                // ch forName spNode refType sibSp fact 1.6", is that part of the node's width.
                struct RingPlace
                {
                    OUString aName;
                    double fWidth; // of the node's width
                    double fHeight; // of the node's width
                    bool bEllipse;
                };
                std::vector<RingPlace> aPlaces;
                bool bBoxes(false);
                OUString aNodeName;
                for (const auto& rCycleChild : aCycleChildren)
                {
                    if (aSpansTheCircle(rCycleChild) || rCycleChild->getSubType() == XML_conn)
                        continue;
                    const bool bSpacer(rCycleChild->getServiceName()
                                       == "com.sun.star.drawing.GroupShape");
                    if (!bSpacer
                        && rCycleChild->getCustomShapeProperties()->getShapePresetType()
                               != XML_ellipse)
                        bBoxes = true;
                    if (aNodeName.isEmpty() && !bSpacer)
                        aNodeName = rCycleChild->getInternalName();
                    aPlaces.push_back({ rCycleChild->getInternalName(), 1.0, 1.0,
                                        !bSpacer
                                            && rCycleChild->getCustomShapeProperties()
                                                       ->getShapePresetType()
                                                   == XML_ellipse });
                }
                if (fSiblingPart != 0.0 && aPlaces.size() >= 2 && !aNodeName.isEmpty())
                {
                    for (const Constraint& rConstraint : rConstraints)
                        if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_sibSp
                            && rConstraint.mnFor == XML_ch && !rConstraint.msForName.isEmpty()
                            && rConstraint.mfFactor > 0.0
                            && !aWidthOf.count(rConstraint.msForName))
                            aWidthOf[rConstraint.msForName]
                                = rConstraint.mfFactor * fSiblingPart;
                    const auto aAspect = aHeightOfWidth.find(aNodeName);
                    const double fHeightOfWidth(aAspect == aHeightOfWidth.end() ? 1.0
                                                                                 : aAspect->second);
                    const auto aNodePart = aWidthOf.find(aNodeName);
                    const double fNodeWidth(aNodePart == aWidthOf.end() ? 1.0
                                                                          : aNodePart->second);
                    for (RingPlace& rPlace : aPlaces)
                    {
                        const auto aPart = aWidthOf.find(rPlace.aName);
                        rPlace.fWidth = (aPart == aWidthOf.end() ? 1.0 : aPart->second)
                                        / fNodeWidth;
                        const auto aOwnAspect = aHeightOfWidth.find(rPlace.aName);
                        rPlace.fHeight = aOwnAspect == aHeightOfWidth.end()
                                             ? (nPlainWidth > 0 ? static_cast<double>(nPlainHeight)
                                                                      / nPlainWidth
                                                                : 1.0)
                                             : rPlace.fWidth * aOwnAspect->second;
                    }
                    // The ring is the largest any two places that follow each other ask for:
                    // from where the line between their middles leaves the one to where it
                    // enters the other, plus the sibSp. An ellipse reaches half its width
                    // whichever way. The line leaves a box through whichever side it meets
                    // first, and that can be further out than half the width: at 25.7 degrees,
                    // the first pair of seven nodes, it leaves through the side at 0.555 of
                    // the width, and the ring of seven is 1.352 widths by that. With a spacer
                    // between two nodes the places are twice the nodes, 45 degrees apart on
                    // Nondirectional_Cycle, and the node's leave at 22.5 degrees, 0.541, the
                    // spacer's 0.13 and the sibSp 0.15 make a ring of 1.073 widths, the
                    // drawing's 2079658 for 1936750.
                    const auto aLeaveOf = [](const RingPlace& rPlace, double fAlong) {
                        if (rPlace.bEllipse)
                            return std::min(rPlace.fWidth, rPlace.fHeight) / 2.0;
                        double fLeave(std::hypot(rPlace.fWidth / 2.0, rPlace.fHeight / 2.0));
                        if (std::abs(cos(fAlong)) > 0.001)
                            fLeave = std::min(fLeave, rPlace.fWidth / 2.0 / std::abs(cos(fAlong)));
                        if (std::abs(sin(fAlong)) > 0.001)
                            fLeave = std::min(fLeave, rPlace.fHeight / 2.0 / std::abs(sin(fAlong)));
                        return fLeave;
                    };
                    // on a part of the ring the first and the last place are no neighbours, so
                    // the span is shared between one gap less
                    const size_t nGaps(bWholeRing ? aPlaces.size() : aPlaces.size() - 1);
                    const double fPlaceTurn(static_cast<double>(nSpanAngle) / nGaps);
                    const double fPlaceStep(basegfx::deg2rad(std::abs(fPlaceTurn)));
                    double fRingOfWidth(0.0);
                    for (size_t nPlace = 0; nPlace < nGaps; ++nPlace)
                    {
                        const double fFrom(basegfx::deg2rad(nPlace * fPlaceTurn + nStartAngle));
                        const double fTo(
                            basegfx::deg2rad((nPlace + 1) * fPlaceTurn + nStartAngle));
                        const double fAlong(std::atan2(-cos(fTo) + cos(fFrom),
                                                       sin(fTo) - sin(fFrom)));
                        const RingPlace& rNext(aPlaces[(nPlace + 1) % aPlaces.size()]);
                        fRingOfWidth = std::max(fRingOfWidth,
                                                (aLeaveOf(aPlaces[nPlace], fAlong)
                                                 + aLeaveOf(rNext, fAlong) + fSiblingPart)
                                                    / (2.0 * sin(fPlaceStep / 2.0)));
                    }
                    if (fRingOfWidth <= 0.0)
                        fRingOfWidth = 0.5 / sin(fPlaceStep / 2.0);
                    // The ring with its places fits the room by each place's own size at its
                    // angle: Text_Cycle's spacers of 0.42 of a node stand at the top and the
                    // bottom, its nodes at the corners, so the ring reaches further than a
                    // node at the top would let it, 1.2 widths for a node of 5325.
                    double fLeftMost(0.0), fRightMost(0.0), fTopMost(0.0), fBottomMost(0.0);
                    for (size_t nPlace = 0; nPlace < aPlaces.size(); ++nPlace)
                    {
                        const double fAt(basegfx::deg2rad(nPlace * fPlaceTurn + nStartAngle));
                        const double fX(fRingOfWidth * sin(fAt));
                        const double fY(-fRingOfWidth * cos(fAt));
                        const double fHalfWidth(aPlaces[nPlace].fWidth / 2.0);
                        const double fHalfHeight(aPlaces[nPlace].fHeight * fHeightOfWidth / 2.0);
                        if (nPlace == 0)
                        {
                            fLeftMost = fX - fHalfWidth;
                            fRightMost = fX + fHalfWidth;
                            fTopMost = fY - fHalfHeight;
                            fBottomMost = fY + fHalfHeight;
                            continue;
                        }
                        fLeftMost = std::min(fLeftMost, fX - fHalfWidth);
                        fRightMost = std::max(fRightMost, fX + fHalfWidth);
                        fTopMost = std::min(fTopMost, fY - fHalfHeight);
                        fBottomMost = std::max(fBottomMost, fY + fHalfHeight);
                    }
                    double fWidth(rShape->getSize().Width / (fReachX * fRingOfWidth + 1.0));
                    if (fReachY > 0.01)
                        fWidth = std::min(fWidth, rShape->getSize().Height
                                                      / (fReachY * fRingOfWidth + fHeightOfWidth));
                    if (!aPlaces.empty() && fRightMost - fLeftMost > 0.01
                        && fBottomMost - fTopMost > 0.01)
                        fWidth = std::min(rShape->getSize().Width / (fRightMost - fLeftMost),
                                          rShape->getSize().Height / (fBottomMost - fTopMost));
                    if (fWidth > 1.0)
                    {
                        nPlainWidth = static_cast<sal_Int32>(fWidth);
                        nPlainHeight = static_cast<sal_Int32>(fWidth * fHeightOfWidth);
                        aChildSize = aLargestChild();
                        fRadius = fRingOfWidth * fWidth;
                        if (bBoxes)
                            fBoxRingSibling = fSiblingPart;
                    }
                }
            }

            // a ring with a single node and nothing in the middle is that node alone, as large
            // as the room lets it be with its own proportions, in the middle
            if (!bMiddleCounts && nPlacesOnTheCircle == 1)
            {
                for (const auto& rCycleChild : aCycleChildren)
                {
                    if (aSpansTheCircle(rCycleChild) || rCycleChild->getSubType() == XML_conn
                        || rCycleChild->getServiceName() == "com.sun.star.drawing.GroupShape")
                        continue;
                    const OUString& rName(rCycleChild->getInternalName());
                    const auto aAspect = aHeightOfWidth.find(rName);
                    const double fHeightOfWidth(aAspect == aHeightOfWidth.end()
                                                    ? static_cast<double>(nPlainHeight)
                                                          / std::max<sal_Int32>(nPlainWidth, 1)
                                                    : aAspect->second);
                    double fWidth(rShape->getSize().Width);
                    if (fHeightOfWidth > 0.0)
                        fWidth = std::min(fWidth, rShape->getSize().Height / fHeightOfWidth);
                    const auto aPart = aWidthOf.find(rName);
                    const double fPart(aPart == aWidthOf.end() || aPart->second <= 0.0
                                           ? 1.0
                                           : aPart->second);
                    nPlainWidth = static_cast<sal_Int32>(fWidth / fPart);
                    nPlainHeight = static_cast<sal_Int32>(fWidth * fHeightOfWidth);
                    aChildSize = aLargestChild();
                    fRadius = 0.0;
                    break;
                }
            }

            // A layout can state the ring outright, diam as a part of the width or the height
            // of the node, or as a length. Then that is the ring, whether it fits the room or
            // not: an arc of a quarter circle stands in a column a fraction as wide as its ring.
            for (const Constraint& rConstraint : rConstraints)
            {
                if (rConstraint.mnType != XML_diam
                    || (!rConstraint.msForName.isEmpty()
                        && rConstraint.msForName != rShape->getInternalName())
                    || !rConstraint.msRefForName.isEmpty())
                    continue;

                double fDiameter(0.0);
                if (rConstraint.mnRefType == XML_h)
                    fDiameter = rShape->getSize().Height * rConstraint.mfFactor;
                else if (rConstraint.mnRefType == XML_w)
                    fDiameter = rShape->getSize().Width * rConstraint.mfFactor;
                else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 1.0)
                    fDiameter = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm,
                                              o3tl::Length::emu);
                if (fDiameter > 1.0)
                    fRadius = fDiameter / 2.0;
            }
            const sal_Int32 nRadius(static_cast<sal_Int32>(fRadius));

            const awt::Size aRingCentre(
                aMiddleAt ? *aMiddleAt
                          : awt::Size(static_cast<sal_Int32>(
                                          aCenter.Width - (fMost + fLeast) / 2.0 * fRadius),
                                      static_cast<sal_Int32>(
                                          aCenter.Height - (fDown + fUp) / 2.0 * fRadius)));

            awt::Size aCentreSize(0, 0);
            OUString aCentreName;
            oox::drawingml::ShapePtr pMiddleShape;
            if (nctrShpMap == XML_fNode)
            {
                // first node placed in center, others around
                oox::drawingml::ShapePtr pCenterShape = aCycleChildren.front();
                aCycleChildren.erase(aCycleChildren.begin());
                aCentreName = pCenterShape->getInternalName();
                pMiddleShape = pCenterShape;
                aCentreSize
                    = aSizeOfChild(pCenterShape->getInternalName(), nPlainWidth, nPlainHeight);
                const awt::Point aCurrPos(aRingCentre.Width - aCentreSize.Width / 2,
                                          aRingCentre.Height - aCentreSize.Height / 2);
                pCenterShape->setPosition(aCurrPos);
                pCenterShape->setSize(aCentreSize);
                pCenterShape->setChildSize(aCentreSize);
            }

            const sal_Int32 nShapes = nPlacesOnTheCircle;
            if (nShapes)
            {
                const sal_Int32 nConnectorRadius = nRadius * cos(basegfx::deg2rad(fStep));
                const sal_Int32 nConnectorAngle = nSpanAngle > 0 ? 0 : 180;

                sal_Int32 idx = 0;
                for (auto & aCurrShape : aCycleChildren)
                {
                    const bool bSpansTheCircle(aSpansTheCircle(aCurrShape));
                    const double fAngle = idx * fStep + nStartAngle;
                    awt::Size aCurrSize
                        = aSizeOfChild(aCurrShape->getInternalName(), nPlainWidth, nPlainHeight);
                    sal_Int32 nCurrRadius = nRadius;
                    // the middle of a block arrow bent round the ring, which stands apart from
                    // the ring's middle
                    std::optional<awt::Size> aArrowMiddle;

                    if (aIsSpoke(aCurrShape))
                    {
                        // The spoke runs from the edge of the middle shape to the node at the
                        // same angle, which is the node that comes next and takes this place.
                        // It ends at the edge of that node, or at its middle where endPts says
                        // ctr, then it runs on under the node and what shows is the part up to
                        // the node's edge. begPad, a part of the whole way, is left free at the
                        // start. It lies flat before it is turned, its width the length of the
                        // way, and it is turned to point outwards.
                        const double fSin(sin(basegfx::deg2rad(fAngle)));
                        const double fCos(cos(basegfx::deg2rad(fAngle)));
                        oox::drawingml::ShapePtr pJoined;
                        for (auto aNext = std::find(aCycleChildren.begin(), aCycleChildren.end(),
                                                    aCurrShape);
                             aNext != aCycleChildren.end(); ++aNext)
                            if (*aNext != aCurrShape && (*aNext)->getSubType() != XML_conn)
                            {
                                pJoined = *aNext;
                                break;
                            }
                        const double fCentreEdge(
                            pMiddleShape
                                ? aEdgeAlong(pMiddleShape, aCentreSize, basegfx::deg2rad(fAngle))
                                : std::abs(fSin) * aCentreSize.Width / 2
                                      + std::abs(fCos) * aCentreSize.Height / 2);
                        const double fNodeEdge(
                            pJoined ? aEdgeAlong(pJoined,
                                                 aSizeOfChild(pJoined->getInternalName(),
                                                              nPlainWidth, nPlainHeight),
                                                 basegfx::deg2rad(fAngle))
                                    : std::abs(fSin) * aChildSize.Width / 2
                                          + std::abs(fCos) * aChildSize.Height / 2);
                        const auto aSites = aEndSites.find(aCurrShape->getInternalName());
                        const bool bToTheMiddle(aSites != aEndSites.end()
                                                && aSites->second.second == XML_ctr);
                        const double fWhole(std::max(
                            1.0, nRadius - fCentreEdge - (bToTheMiddle ? 0.0 : fNodeEdge)));
                        const auto aPad = aBeginPads.find(aCurrShape->getInternalName());
                        double fPad(aPad != aBeginPads.end() ? aPad->second * fWhole : 0.0);
                        double fEndPad(0.0);
                        // A spoke that states no pads at all leaves 0.47 of its way free, as any
                        // connector that states none: Diverging_Radial's arrows are 369673 of a
                        // way of 697499 between the middle's edge and the node's.
                        if (!aPadsStated.count(aCurrShape->getInternalName()))
                            fPad = fEndPad = 0.235 * fWhole;
                        const double fWay(std::max(1.0, fWhole - fPad - fEndPad));
                        const double fMiddle(fCentreEdge + fPad + fWay / 2);
                        // As thick as the line says it is, or as the ring says for it, h as a
                        // part of the width of a named shape on the ring, the middle one for a
                        // fat arrow, or a twelfth of the room like any other connector of the
                        // ring where nothing says anything. A stated thickness is a length in
                        // millimetres at the plain size, where the middle shape has the width
                        // the ring states for it, and it shrinks as the middle shape did:
                        // Radial_List's spoke of 5 becomes 44208 EMU for a middle of 0.2456 of
                        // the width it was given.
                        const auto aThick = aStatedHeight.find(aCurrShape->getInternalName());
                        double fFixedScale(1.0);
                        if (aThick != aStatedHeight.end() && !aCentreName.isEmpty()
                            && rShape->getSize().Width > 0)
                        {
                            // on a ring with a spacing the parts are of the plain quarter, so
                            // a quarter of one is the part of the ring's width
                            const auto aPart = aWidthOf.find(aCentreName);
                            const double fStated(rShape->getSize().Width
                                                 * (aPart == aWidthOf.end() ? 1.0
                                                    : bStatesASpacing ? aPart->second / 4.0
                                                                      : aPart->second));
                            if (fStated > 0.0)
                                fFixedScale = aCentreSize.Width / fStated;
                        }
                        sal_Int32 nThick(
                            aThick != aStatedHeight.end()
                                ? static_cast<sal_Int32>(aThick->second * fFixedScale)
                            : aLinesOfNothing.count(aCurrShape->getInternalName())
                                ? o3tl::convert(1, o3tl::Length::mm100, o3tl::Length::emu)
                                : aSizeOfChild(aCurrShape->getInternalName(),
                                               aConnectorSize.Width, aConnectorSize.Height)
                                      .Height);
                        for (const Constraint& rConstraint : rConstraints)
                        {
                            if (rConstraint.mnType != XML_h || rConstraint.mnRefType != XML_w
                                || rConstraint.msRefForName.isEmpty()
                                || rConstraint.mfFactor <= 0.0
                                || rConstraint.msForName != aCurrShape->getInternalName())
                                continue;
                            const sal_Int32 nOfWidth(
                                rConstraint.msRefForName == aCentreName
                                    ? aCentreSize.Width
                                    : aSizeOfChild(rConstraint.msRefForName, nPlainWidth,
                                                   nPlainHeight)
                                          .Width);
                            if (nOfWidth > 0)
                                nThick = static_cast<sal_Int32>(nOfWidth * rConstraint.mfFactor);
                        }
                        // A spoke whose width the ring states and whose height is a part of its
                        // own width is that part of the stated width thick, whatever of its way
                        // it draws: Diverging_Radial's arrows are 0.85 of 0.4 of the middle,
                        // 593724, drawn over 369673 of the way. The connector's own algorithm
                        // would take the part of the drawn length, so the spoke is kept as it is.
                        bool bThickByStatedWidth(false);
                        if (aThick == aStatedHeight.end()
                            && !aLinesOfNothing.count(aCurrShape->getInternalName())
                            && aWidthOf.count(aCurrShape->getInternalName())
                            && aHeightOfWidth.count(aCurrShape->getInternalName()))
                        {
                            nThick = aSizeOfChild(aCurrShape->getInternalName(), nPlainWidth,
                                                  nPlainHeight)
                                         .Height;
                            bThickByStatedWidth = true;
                        }
                        const awt::Size aSpokeSize(static_cast<sal_Int32>(fWay), nThick);
                        if (bThickByStatedWidth)
                            const_cast<SmartArtDiagram&>(rDgm).getLaidOutSideways().insert(
                                aCurrShape.get());
                        const awt::Point aSpokePos(
                            aRingCentre.Width + fMiddle * fSin - aSpokeSize.Width / 2,
                            aRingCentre.Height - fMiddle * fCos - aSpokeSize.Height / 2);
                        aCurrShape->setPosition(aSpokePos);
                        aCurrShape->setSize(aSpokeSize);
                        aCurrShape->setChildSize(aSpokeSize);
                        aCurrShape->setRotation(
                            static_cast<sal_Int32>((fAngle - 90.0) * PER_DEGREE));
                        continue;
                    }

                    if (bSpansTheCircle)
                    {
                        // As large as the ring, the circle through the middles of the nodes,
                        // and in the middle of them. What such a shape draws is an arc that
                        // runs round through all of them. Where the layout states the diam of
                        // the cycle itself as a bare 1 with nothing to refer to, "diam val=1",
                        // the arc is as large as the room lets a circle be; that is read off
                        // Text_Cycle, whose nodes stand on a smaller ring than its arrow draws.
                        // A constraint of the node's own with no name is not among the ones
                        // handed down, so the node's own are read for it, the way the layout
                        // read them for this node's point, a choose in between decided.
                        sal_Int32 nSpan(nRadius * 2);
                        std::vector<Constraint> aOwn;
                        {
                            rtl::Reference<svx::diagram::Point> xOwn;
                            for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                                if (rEntry.second == rShape)
                                {
                                    xOwn = rEntry.first;
                                    break;
                                }
                            gatherDecidedConstraints(rDgm, getLayoutNode(), xOwn, aOwn);
                        }
                        for (const Constraint& rConstraint : aOwn)
                            if (rConstraint.mnType == XML_diam && rConstraint.msForName.isEmpty()
                                && rConstraint.mnRefType == XML_none
                                && rConstraint.mfValue > 0.0 && rConstraint.mfValue <= 1.0)
                                nSpan = static_cast<sal_Int32>(
                                    std::min(rShape->getSize().Width, rShape->getSize().Height)
                                    * rConstraint.mfValue);
                        const AlgAtom* pOwnAlg(nullptr);
                        {
                            std::map<OUString, const LayoutNode*> aRingNodes;
                            gatherLayoutNodes(mrLayoutNode, aRingNodes);
                            const auto aNode = aRingNodes.find(aCurrShape->getInternalName());
                            rtl::Reference<svx::diagram::Point> xChildPoint;
                            for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                                if (rEntry.second == aCurrShape)
                                {
                                    xChildPoint = rEntry.first;
                                    break;
                                }
                            if (aNode != aRingNodes.end() && aNode->second)
                                pOwnAlg = algorithmOf(rDgm, *aNode->second, xChildPoint);
                        }
                        const auto aOfRing = aSpanOfRing.find(aCurrShape.get());
                        const auto aRoute
                            = pOwnAlg ? pOwnAlg->getMap().find(XML_connRout)
                                      : std::map<sal_Int32, sal_Int32>::const_iterator();
                        const bool bCurved(pOwnAlg && pOwnAlg->getType() == XML_conn
                                           && aRoute != pOwnAlg->getMap().end()
                                           && (aRoute->second == XML_curve
                                               || aRoute->second == XML_longCurve));
                        if (aOfRing != aSpanOfRing.end())
                        {
                            const RingArrow& rArrow(aOfRing->second);
                            nSpan = static_cast<sal_Int32>(
                                nRadius * 2 * (rArrow.mfCirclePart + 2.0 * rArrow.mfStem));
                            // The arrow's circle, the middle line of its band, is the stated
                            // part of the ring's diameter, 1.04 for five nodes, and it runs
                            // through the middles of the two sides of the first node, so its
                            // middle stands below the node's middle by what a chord of the
                            // node's width leaves of that radius. The band is stemThick of the
                            // ring's diameter thick, 0.065, and the head reaches hArH of it,
                            // 0.1, from the middle line outwards, the way the preset takes it,
                            // so the box is twice the circle and the head's reach; the circle
                            // in that box comes out 0.9976 of the stated one, in every one of
                            // the eight files. The arrow starts at the site the layout names
                            // on the first node, midR for the norm direction, less the begPad
                            // of the way between two nodes, 0.2 of a node's width, turned into
                            // an angle on the circle, and its head ends short of the other
                            // site by the endPad and the head's length, wArH of the ring's
                            // diameter. The angles of the preset run clockwise from the right,
                            // and a diam of the other sign runs the arrow the other way round,
                            // as the left circular arrow preset with the same angles mirrored.
                            const double fRingDiameter(2.0 * nRadius);
                            const double fCircle(rArrow.mfCirclePart * nRadius);
                            const double fStem(rArrow.mfStem * fRingDiameter);
                            const double fReach(rArrow.mfHeadReach * fRingDiameter - fStem / 2.0);
                            OUString aFirstNode;
                            for (const auto& rNode : aCycleChildren)
                                if (rNode->getSubType() != XML_conn && !aSpansTheCircle(rNode)
                                    && !aIsSpoke(rNode))
                                {
                                    aFirstNode = rNode->getInternalName();
                                    break;
                                }
                            if (bCurved && fCircle > 1.0 && fReach > 0.0 && fStem > 0.0
                                && !aFirstNode.isEmpty())
                            {
                                const awt::Size aNode(
                                    aSizeOfChild(aFirstNode, nPlainWidth, nPlainHeight));
                                const double fNodeAngle(basegfx::deg2rad(nStartAngle));
                                const awt::Point aNodeAt(
                                    static_cast<sal_Int32>(aRingCentre.Width
                                                           + nRadius * sin(fNodeAngle))
                                        - aNode.Width / 2,
                                    static_cast<sal_Int32>(aRingCentre.Height
                                                           - nRadius * cos(fNodeAngle))
                                        - aNode.Height / 2);
                                const double fNodeMiddleY(aNodeAt.Y + aNode.Height / 2.0);
                                const double fHalfWidth(aNode.Width / 2.0);
                                const double fBelow(std::sqrt(
                                    std::max(0.0, fCircle * fCircle - fHalfWidth * fHalfWidth)));
                                const double fSide(aRingCentre.Height >= fNodeMiddleY ? 1.0
                                                                                        : -1.0);
                                const double fMiddleX(aNodeAt.X + aNode.Width / 2.0);
                                const double fMiddleY(fNodeMiddleY + fSide * fBelow);
                                const auto& rMap = pOwnAlg->getMap();
                                const sal_Int32 nBeginSite(
                                    rMap.count(XML_begPts)
                                        ? rMap.find(XML_begPts)->second
                                        : (rArrow.mbClockwise ? XML_midR : XML_midL));
                                const sal_Int32 nEndSite(
                                    rMap.count(XML_endPts)
                                        ? rMap.find(XML_endPts)->second
                                        : (rArrow.mbClockwise ? XML_midL : XML_midR));
                                const auto aAngleOf = [fMiddleX, fMiddleY](const awt::Point& rAt) {
                                    return basegfx::rad2deg(
                                        std::atan2(rAt.Y - fMiddleY, rAt.X - fMiddleX));
                                };
                                const double fTurn(rArrow.mbClockwise ? 1.0 : -1.0);
                                const double fWay(aNode.Width);
                                double fStart(aAngleOf(siteOn(nBeginSite, aNodeAt, aNode))
                                              + fTurn
                                                    * basegfx::rad2deg(rArrow.mfBeginPad * fWay
                                                                       / fCircle));
                                double fEnd(aAngleOf(siteOn(nEndSite, aNodeAt, aNode))
                                            - fTurn
                                                  * basegfx::rad2deg(
                                                      (rArrow.mfEndPad * fWay
                                                       + rArrow.mfHeadLength * fRingDiameter)
                                                      / fCircle));
                                const double fHeadAngle(basegfx::rad2deg(
                                    rArrow.mfHeadLength * fRingDiameter / fCircle));
                                fStart = std::fmod(fStart + 720.0, 360.0);
                                fEnd = std::fmod(fEnd + 720.0, 360.0);
                                const double fBox(2.0 * (0.9976 * fCircle + fReach));
                                nSpan = static_cast<sal_Int32>(fBox);
                                aArrowMiddle = awt::Size(static_cast<sal_Int32>(fMiddleX),
                                                         static_cast<sal_Int32>(fMiddleY));

                                const sal_Int32 nPreset(rArrow.mbClockwise
                                                            ? XML_circularArrow
                                                            : XML_leftCircularArrow);
                                aCurrShape->setSubType(nPreset);
                                auto& rProperties(*aCurrShape->getCustomShapeProperties());
                                rProperties.setShapePresetType(nPreset);
                                auto& rGuides(rProperties.getAdjustmentGuideList());
                                const auto aSet = [&rGuides](const OUString& rName, double fValue) {
                                    setGuideValue(rGuides, rName,
                                                  static_cast<sal_Int32>(std::lround(fValue)));
                                };
                                aSet(u"adj1"_ustr, fStem / fBox * 100000.0);
                                aSet(u"adj2"_ustr, fHeadAngle * 60000.0);
                                aSet(u"adj3"_ustr, fEnd * 60000.0);
                                aSet(u"adj4"_ustr, fStart * 60000.0);
                                aSet(u"adj5"_ustr, fReach / fBox * 100000.0);
                                // the arrow is settled here, so the connector's own algorithm
                                // leaves it as it is
                                const_cast<SmartArtDiagram&>(rDgm).getLaidOutSideways().insert(
                                    aCurrShape.get());
                            }
                        }
                        // A connector with a body, dim 2D, a bent block arrow round the ring,
                        // reaches 1.3 of its thickness beyond the ring's circle, its head wider
                        // than its band: Block_Cycle2D's arrows are 4650280 across for a ring of
                        // 4159314, the arrow 0.3 of the node wide and 0.65 of that thick, 377666.
                        if (!aArrowMiddle)
                        {
                            const auto aDim = pOwnAlg ? pOwnAlg->getMap().find(XML_dim)
                                                      : std::map<sal_Int32, sal_Int32>::const_iterator();
                            if (pOwnAlg && pOwnAlg->getType() == XML_conn
                                && aDim != pOwnAlg->getMap().end() && aDim->second == XML_2D)
                            {
                                // the thickness is a part of the stated width, which the ring
                                // states as a part of a node's, by name or for every sibTrans
                                sal_Int32 nThick(0);
                                const auto aOwnHeight
                                    = aHeightOfWidth.find(aCurrShape->getInternalName());
                                for (const Constraint& rConstraint : rConstraints)
                                {
                                    if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
                                        || rConstraint.mnFor != XML_ch
                                        || rConstraint.mnRefFor != XML_ch
                                        || rConstraint.mfFactor <= 0.0
                                        || aOwnHeight == aHeightOfWidth.end())
                                        continue;
                                    const bool bForThis(
                                        rConstraint.msForName == aCurrShape->getInternalName()
                                        || (rConstraint.msForName.isEmpty()
                                            && rConstraint.mnPointType == XML_sibTrans));
                                    if (!bForThis)
                                        continue;
                                    OUString aOfNode(rConstraint.msRefForName);
                                    if (aOfNode.isEmpty() && rConstraint.mnRefPointType == XML_node)
                                        for (const auto& rNode : aCycleChildren)
                                            if (rNode->getSubType() != XML_conn
                                                && !aSpansTheCircle(rNode))
                                            {
                                                aOfNode = rNode->getInternalName();
                                                break;
                                            }
                                    if (aOfNode.isEmpty())
                                        continue;
                                    nThick = static_cast<sal_Int32>(
                                        aSizeOfChild(aOfNode, nPlainWidth, nPlainHeight).Width
                                        * rConstraint.mfFactor * aOwnHeight->second);
                                    break;
                                }
                                if (nThick > 0 && nThick < nSpan)
                                    nSpan += static_cast<sal_Int32>(1.3 * nThick);
                            }
                        }
                        aCurrSize = awt::Size(nSpan, nSpan);
                        nCurrRadius = 0;
                    }
                    else if (aCurrShape->getSubType() == XML_conn)
                    {
                        aCurrSize = aSizeOfChild(aCurrShape->getInternalName(),
                                                 aConnectorSize.Width, aConnectorSize.Height);
                        // The ring may state the connector's width as a part of a node's, by
                        // the connector's name or for every sibTrans, "w for ch ptType sibTrans
                        // refType w refFor ch refPtType node fact 0.25" on Basic_Cycle: then it
                        // is that part of the node as laid out, 433586 of 1734343, and its
                        // height a part of that width where it says so, 1.35 there.
                        for (const Constraint& rConstraint : rConstraints)
                        {
                            if (rConstraint.mnType != XML_w || rConstraint.mnRefType != XML_w
                                || rConstraint.mnFor != XML_ch || rConstraint.mnRefFor != XML_ch
                                || rConstraint.mfFactor <= 0.0)
                                continue;
                            const bool bForThis(
                                rConstraint.msForName == aCurrShape->getInternalName()
                                || (rConstraint.msForName.isEmpty()
                                    && rConstraint.mnPointType == XML_sibTrans));
                            if (!bForThis)
                                continue;
                            OUString aOfNode(rConstraint.msRefForName);
                            if (aOfNode.isEmpty() && rConstraint.mnRefPointType == XML_node)
                                for (const auto& rNode : aCycleChildren)
                                    if (rNode->getSubType() != XML_conn
                                        && !aSpansTheCircle(rNode))
                                    {
                                        aOfNode = rNode->getInternalName();
                                        break;
                                    }
                            if (aOfNode.isEmpty())
                                continue;
                            const awt::Size aNode(aSizeOfChild(aOfNode, nPlainWidth, nPlainHeight));
                            aCurrSize.Width = static_cast<sal_Int32>(aNode.Width * rConstraint.mfFactor);
                            const auto aOwnHeight
                                = aHeightOfWidth.find(aCurrShape->getInternalName());
                            if (aOwnHeight != aHeightOfWidth.end())
                                aCurrSize.Height
                                    = static_cast<sal_Int32>(aCurrSize.Width * aOwnHeight->second);
                            // Between two boxes that stand the sibSp apart the connector runs
                            // over that room less its pads, parts of the way, a tenth at each
                            // end on Multidirectional_Cycle, 1166145 of 1452364; it stays as
                            // thick as the part of its stated width says.
                            if (fBoxRingSibling > 0.0)
                            {
                                const double fWhole(aNode.Width * fBoxRingSibling);
                                double fPads(0.47);
                                if (aPadsStated.count(aCurrShape->getInternalName()))
                                {
                                    fPads = 0.0;
                                    std::map<OUString, const LayoutNode*> aRingNodes;
                                    gatherLayoutNodes(mrLayoutNode, aRingNodes);
                                    const auto aNodeOf
                                        = aRingNodes.find(aCurrShape->getInternalName());
                                    std::vector<Constraint> aOwn;
                                    if (aNodeOf != aRingNodes.end() && aNodeOf->second)
                                        gatherOwnConstraints(*aNodeOf->second, aOwn);
                                    for (const Constraint& rOwn : aOwn)
                                        if ((rOwn.mnType == XML_begPad || rOwn.mnType == XML_endPad)
                                            && rOwn.mnRefType == XML_connDist)
                                            fPads += rOwn.mfFactor;
                                }
                                aCurrSize.Width = static_cast<sal_Int32>(
                                    std::max(1.0, fWhole * (1.0 - fPads)));
                                // the thickness is settled here, a part of the stated width, so
                                // the connector's own algorithm leaves the shape as it is
                                const_cast<SmartArtDiagram&>(rDgm).getLaidOutSideways().insert(
                                    aCurrShape.get());
                            }
                            break;
                        }
                        nCurrRadius = nConnectorRadius;
                    }
                    const awt::Point aCurrPos(
                        aArrowMiddle ? aArrowMiddle->Width - aCurrSize.Width / 2
                                     : aRingCentre.Width
                                           + nCurrRadius * sin(basegfx::deg2rad(fAngle))
                                           - aCurrSize.Width / 2,
                        aArrowMiddle ? aArrowMiddle->Height - aCurrSize.Height / 2
                                     : aRingCentre.Height
                                           - nCurrRadius * cos(basegfx::deg2rad(fAngle))
                                           - aCurrSize.Height / 2);

                    aCurrShape->setPosition(aCurrPos);
                    aCurrShape->setSize(aCurrSize);
                    aCurrShape->setChildSize(aCurrSize);

                    // a curved connector round the whole ring with no head at either end (begSty
                    // and endSty noArr) is a band from the node before it to the next one, the
                    // blockArc preset, 0.6 of the stated h; a ring turning the other way runs it
                    // back
                    if (bSpansTheCircle && !aArrowMiddle && aCurrShape->getSubType() == XML_conn
                        && aCurrSize.Width > 0)
                    {
                        std::map<OUString, const LayoutNode*> aArcNodes;
                        gatherLayoutNodes(mrLayoutNode, aArcNodes);
                        const auto aArcNode = aArcNodes.find(aCurrShape->getInternalName());
                        rtl::Reference<svx::diagram::Point> xArcPoint;
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                            if (rEntry.second == aCurrShape)
                            {
                                xArcPoint = rEntry.first;
                                break;
                            }
                        const AlgAtom* pArcAlgorithm(
                            aArcNode != aArcNodes.end() && aArcNode->second
                                ? algorithmOf(rDgm, *aArcNode->second, xArcPoint)
                                : nullptr);
                        const auto aParameter = [pArcAlgorithm](sal_Int32 nParameter,
                                                                sal_Int32 nUnstated) {
                            const auto aFound = pArcAlgorithm->getMap().find(nParameter);
                            return aFound != pArcAlgorithm->getMap().end() ? aFound->second
                                                                           : nUnstated;
                        };
                        if (pArcAlgorithm && pArcAlgorithm->getType() == XML_conn
                            && (aParameter(XML_connRout, XML_stra) == XML_curve
                                || aParameter(XML_connRout, XML_stra) == XML_longCurve)
                            && aParameter(XML_begSty, XML_noArr) == XML_noArr
                            && aParameter(XML_endSty, XML_arr) == XML_noArr)
                        {
                            double fBand(0.0);
                            for (const Constraint& rConstraint : rConstraints)
                                if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w
                                    && rConstraint.msForName == aCurrShape->getInternalName()
                                    && !rConstraint.msRefForName.isEmpty()
                                    && rConstraint.mfFactor > 0.0)
                                    fBand = 0.6 * rConstraint.mfFactor
                                            * aSizeOfChild(rConstraint.msRefForName, nPlainWidth,
                                                           nPlainHeight)
                                                  .Width;
                            if (fBand > 0.0)
                            {
                                // the preset's angles run clockwise from the right, the ring's
                                // from the top
                                aCurrShape->setSubType(XML_blockArc);
                                // each arc has angles of its own, and the transitions of a ring
                                // share one set of properties
                                aCurrShape->getCustomShapeProperties()
                                    = std::make_shared<CustomShapeProperties>(
                                        *aCurrShape->getCustomShapeProperties());
                                auto& rProperties(*aCurrShape->getCustomShapeProperties());
                                rProperties.setShapePresetType(XML_blockArc);
                                auto& rGuides(rProperties.getAdjustmentGuideList());
                                const auto aSet = [&rGuides](const OUString& rName, double fValue) {
                                    setGuideValue(rGuides, rName,
                                                  static_cast<sal_Int32>(std::lround(fValue)));
                                };
                                // the band's box reaches a quarter of the band beyond
                                // the circle through the nodes' middles
                                const sal_Int32 nArcBox(
                                    static_cast<sal_Int32>(2.0 * (nRadius + fBand / 4.0)));
                                aCurrSize = awt::Size(nArcBox, nArcBox);
                                aCurrShape->setSize(aCurrSize);
                                aCurrShape->setChildSize(aCurrSize);
                                aCurrShape->setPosition(
                                    awt::Point(aRingCentre.Width - nArcBox / 2,
                                               aRingCentre.Height - nArcBox / 2));
                                aSet(u"adj3"_ustr, fBand / aCurrSize.Width * 100000.0);
                            }
                        }
                    }

                    if (nRotationPath == XML_alongPath && !bSpansTheCircle)
                        aCurrShape->setRotation(fAngle * PER_DEGREE);

                    // connectors should be handled in conn, but we don't have
                    // reference to previous and next child, so it's easier here
                    if (aCurrShape->getSubType() == XML_conn && !bSpansTheCircle)
                        aCurrShape->setRotation((nConnectorAngle + fAngle) * PER_DEGREE);

                    // a shape that spans the circle sits between the same two nodes as before,
                    // so it takes no place of its own
                    if (!bSpansTheCircle)
                        idx++;
                }

                // once the nodes stand, an arc runs from the middle of the node before it to
                // the middle of the next one; hidden spacers and a middle shape are not on the ring
                const auto aDrawnNode = [&](const oox::drawingml::ShapePtr& rOne) {
                    const double fOffX(rOne->getPosition().X + rOne->getSize().Width / 2.0
                                       - aRingCentre.Width);
                    const double fOffY(rOne->getPosition().Y + rOne->getSize().Height / 2.0
                                       - aRingCentre.Height);
                    return rOne->getServiceName() != "com.sun.star.drawing.GroupShape"
                           && rOne->getSubType() != XML_conn
                           && rOne->getSubType() != XML_blockArc
                           && !aSpansTheCircle(rOne) && !aIsSpoke(rOne)
                           && std::hypot(fOffX, fOffY) > 0.01 * std::max<double>(nRadius, 1.0);
                };
                const auto aAngleOf = [&aRingCentre](const oox::drawingml::ShapePtr& rOne) {
                    const double fX(rOne->getPosition().X + rOne->getSize().Width / 2.0
                                    - aRingCentre.Width);
                    const double fY(rOne->getPosition().Y + rOne->getSize().Height / 2.0
                                    - aRingCentre.Height);
                    return std::fmod(basegfx::rad2deg(std::atan2(fY, fX)) + 360.0, 360.0);
                };
                const size_t nChildren(aCycleChildren.size());
                for (size_t nArc = 0; nArc < nChildren; ++nArc)
                {
                    const oox::drawingml::ShapePtr& rArc(aCycleChildren[nArc]);
                    if (rArc->getSubType() != XML_blockArc)
                        continue;
                    oox::drawingml::ShapePtr pFrom, pTo;
                    for (size_t nStep = 1; nStep < nChildren && !pFrom; ++nStep)
                        if (aDrawnNode(aCycleChildren[(nArc + nChildren - nStep) % nChildren]))
                            pFrom = aCycleChildren[(nArc + nChildren - nStep) % nChildren];
                    for (size_t nStep = 1; nStep < nChildren && !pTo; ++nStep)
                        if (aDrawnNode(aCycleChildren[(nArc + nStep) % nChildren]))
                            pTo = aCycleChildren[(nArc + nStep) % nChildren];
                    if (!pFrom || !pTo || pFrom == pTo)
                        continue;
                    double fFrom(aAngleOf(pFrom)), fTo(aAngleOf(pTo));
                    if (fStep < 0.0)
                        std::swap(fFrom, fTo);
                    auto& rGuides(rArc->getCustomShapeProperties()->getAdjustmentGuideList());
                    const auto aSet = [&rGuides](const OUString& rName, double fValue) {
                        setGuideValue(rGuides, rName, static_cast<sal_Int32>(std::lround(fValue)));
                    };
                    aSet(u"adj1"_ustr, fFrom * 60000.0);
                    aSet(u"adj2"_ustr, fTo * 60000.0);
                }
            }
            break;
        }

        case XML_hierChild:
        case XML_hierRoot:
        {
            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

            // What a sideways branch above has placed as a whole keeps its place.
            if (const_cast<SmartArtDiagram&>(rDgm).getLaidOutSideways().count(rShape.get()))
                break;

            if (mnType == XML_hierChild && layoutSidewaysBranch(rDgm, rShape, rConstraints))
                break;

            if (mnType == XML_hierChild && layoutUprightBranch(rDgm, rShape, rConstraints))
                break;

            if (mnType == XML_hierRoot && layoutSidewaysRoot(rShape, rConstraints))
                break;

            // hierRoot is the manager -> employees vertical linear path,
            // hierChild is the first employee -> last employee horizontal
            // linear path.
            sal_Int32 nDir = XML_fromL;
            if (mnType == XML_hierRoot)
            {
                // The root stands above the branch below it, or beside it where the layout says
                // so. hierAlign names the side the root takes, and a hierarchy that grows across
                // the page says so there and nowhere else.
                nDir = XML_fromT;
                const sal_Int32 nHierAlign(maMap.count(XML_hierAlign)
                                               ? maMap.find(XML_hierAlign)->second
                                               : 0);
                if (nHierAlign == XML_lCtrCh || nHierAlign == XML_lT || nHierAlign == XML_lB)
                    nDir = XML_fromL;
                else if (nHierAlign == XML_rCtrCh || nHierAlign == XML_rT
                         || nHierAlign == XML_rB)
                    nDir = XML_fromR;
            }
            else if (maMap.count(XML_linDir))
                nDir = maMap.find(XML_linDir)->second;

            const sal_Int32 nSecDir = maMap.count(XML_secLinDir) ? maMap.find(XML_secLinDir)->second : 0;

            sal_Int32 nCount = rShape->getChildren().size();

            if (mnType == XML_hierChild)
            {
                // Connectors should not influence the size of non-connect shapes.
                nCount = std::count_if(
                    rShape->getChildren().begin(), rShape->getChildren().end(),
                    [](const ShapePtr& pShape) { return pShape->getSubType() != XML_conn; });
            }

            // The layout states each gap as a part of the shape it sits next to. Where it states
            // none, the gaps keep the shares that the hierarchy layouts have used
            const double fSpaceWidth = readSpacingFactor(rConstraints, XML_w, 0.1);
            double fSpaceHeight = readSpacingFactor(rConstraints, XML_h, 0.3);

            // The children of a hanging branch stand apart by the sibSp stated for the branch,
            // and touch where none is stated; the sp of the root is the root's gap, not theirs.
            const auto aFirstCell = [](const ShapePtr& pOf) {
                for (const ShapePtr& pChild : pOf->getChildren())
                    if (pChild->getSubType() != XML_conn)
                        return pChild;
                return ShapePtr();
            };
            if (mnType == XML_hierChild && (nDir == XML_fromT || nDir == XML_fromB))
                if (const ShapePtr pCell = aFirstCell(rShape))
                    if (const std::optional<HangingWeights> oWeights = readHangingWeights(
                            rConstraints, pCell->getInternalName(), rShape->getInternalName()))
                        fSpaceHeight = oWeights->fChildGapOfChild;

            if (mnType == XML_hierRoot && nCount == 3)
            {
                // Order assistant nodes above employee nodes.
                std::vector<ShapePtr>& rChildren = rShape->getChildren();
                if (!containsDataNodeType(rChildren[1], XML_asst)
                    && containsDataNodeType(rChildren[2], XML_asst))
                    std::swap(rChildren[1], rChildren[2]);
            }

            sal_Int32 nHorizontalShapesCount = 1;
            if (nSecDir == XML_fromT)
                nHorizontalShapesCount = 2;
            else if (nDir == XML_fromL || nDir == XML_fromR)
                nHorizontalShapesCount = nCount;

            awt::Size aChildSize = rShape->getSize();
            aChildSize.Height /= (rShape->getVerticalShapesCount() + (rShape->getVerticalShapesCount() - 1) * fSpaceHeight);
            aChildSize.Width /= (nHorizontalShapesCount + (nHorizontalShapesCount - 1) * fSpaceWidth);

            // A root beside its branch is one column of two, and its width comes from its
            // proportions and not from an even share of the room. So the fit below starts from
            // the whole room and narrows it to the proportions, and the branch takes what is
            // left. An even share first would leave the fit only the height to give, and every
            // level down would halve the rows again.
            if (mnType == XML_hierRoot && (nDir == XML_fromL || nDir == XML_fromR)
                && readHeightOfOwnWidthFactor(rConstraints) > 0.0)
                aChildSize.Width = rShape->getSize().Width;

            // A row that holds a proportion of its own width takes its width from the height of
            // one row. A layout may state a node's two sides as parts of the frame instead,
            // "w for des forName rootComposite refType w fact 4" and "h ... refType h" on
            // Titled_Picture_Accent_List, four widths by the height: the proportions are those
            // parts against the frame, 6 to 1 there, and the drawing scales the whole hierarchy
            // by one factor to composites of 5248 by 875.
            double fHeightOfWidth = readHeightOfOwnWidthFactor(rConstraints);
            if (fHeightOfWidth <= 0.0)
                if (const ShapePtr pCell = aFirstCell(rShape))
                {
                    // the side of the named shape as a part of the frame of the node that
                    // states it, followed through a part of another named shape, "w for des
                    // childComposite refType w refFor des refForName rootComposite"
                    std::function<std::optional<double>(const OUString&, sal_Int32, OUString&,
                                                        int)>
                        aPartOfFrame = [&](const OUString& rName, sal_Int32 nSide,
                                           OUString& rStatedBy,
                                           int nDepth) -> std::optional<double> {
                        if (nDepth > 4)
                            return std::nullopt;
                        for (const Constraint& rConstraint : rConstraints)
                        {
                            if (rConstraint.msForName != rName || rConstraint.mnType != nSide
                                || rConstraint.mnRefType != nSide || rConstraint.mfFactor <= 0.0
                                || rConstraint.mnOperator != XML_none)
                                continue;
                            if (rConstraint.msRefForName.isEmpty())
                            {
                                rStatedBy = rConstraint.msStatedBy;
                                return rConstraint.mfFactor;
                            }
                            const std::optional<double> oOf(aPartOfFrame(
                                rConstraint.msRefForName, nSide, rStatedBy, nDepth + 1));
                            if (oOf)
                                return *oOf * rConstraint.mfFactor;
                        }
                        return std::nullopt;
                    };
                    OUString aStatedBy;
                    const std::optional<double> oWidthPart(
                        aPartOfFrame(pCell->getInternalName(), XML_w, aStatedBy, 0));
                    const std::optional<double> oHeightPart(
                        aPartOfFrame(pCell->getInternalName(), XML_h, aStatedBy, 0));
                    if (oWidthPart && oHeightPart)
                    {
                        const std::optional<sal_Int32> oFrameWidth(
                            sizeOfLaidOutShape(rDgm, aStatedBy, XML_w));
                        const std::optional<sal_Int32> oFrameHeight(
                            sizeOfLaidOutShape(rDgm, aStatedBy, XML_h));
                        if (oFrameWidth && oFrameHeight && *oFrameWidth > 0)
                            fHeightOfWidth = *oHeightPart * *oFrameHeight
                                             / (*oWidthPart * *oFrameWidth);
                    }
                }
            bool bHoldsProportions = false;
            if (fHeightOfWidth > 0.0)
            {
                // Fit the largest box of those proportions into the room there is. Narrowing is
                // not enough on its own: where the room is taller than the proportions allow, it
                // is the height that has to give, and a hierarchy is mostly that way about.
                const sal_Int32 nWidth(static_cast<sal_Int32>(aChildSize.Height / fHeightOfWidth));
                if (nWidth > 0 && nWidth <= aChildSize.Width)
                {
                    aChildSize.Width = nWidth;
                    bHoldsProportions = true;
                }
                else
                {
                    const sal_Int32 nHeight(
                        static_cast<sal_Int32>(aChildSize.Width * fHeightOfWidth));
                    if (nHeight > 0 && nHeight < aChildSize.Height)
                    {
                        aChildSize.Height = nHeight;
                        bHoldsProportions = true;
                    }
                }
            }

            // A root above a hanging branch: the root's cell one unit high, the children's cells
            // their stated part of it each, the gaps between as stated, the whole filling the
            // column. Square_Accent_List has the children 0.5205 of the root and touching.
            std::optional<double> oBranchUnits;
            if (mnType == XML_hierRoot && nDir == XML_fromT && rShape->getChildren().size() == 2)
            {
                const ShapePtr& pBranch(rShape->getChildren()[1]);
                if (const ShapePtr pCell = aFirstCell(pBranch))
                    if (const std::optional<HangingWeights> oWeights = readHangingWeights(
                            rConstraints, pCell->getInternalName(), pBranch->getInternalName()))
                    {
                        const sal_Int32 nBelow(std::max<sal_Int32>(
                            1, pBranch->getVerticalShapesCount()));
                        const double fBelow(nBelow * oWeights->fChildOfRoot
                                            + (nBelow - 1) * oWeights->fChildGapOfChild
                                                  * oWeights->fChildOfRoot);
                        aChildSize.Height = static_cast<sal_Int32>(
                            rShape->getSize().Height / (1.0 + oWeights->fRootGapOfRoot + fBelow));
                        fSpaceHeight = oWeights->fRootGapOfRoot;
                        oBranchUnits = fBelow;

                        // The root's cell holds the proportions the layout states for it, "w
                        // for des rootComposite refType h refFor des rootComposite fact 3.0396",
                        // and the column is only as high as that makes it: Square_Accent_List's
                        // roots are all 644259 high whatever the count of children below them,
                        // and the columns stop short of the height.
                        const OUString& rRootCell(rShape->getChildren()[0]->getInternalName());
                        for (const Constraint& rConstraint : rConstraints)
                        {
                            if (rConstraint.msForName != rRootCell || rConstraint.mfFactor <= 0.0
                                || (!rConstraint.msRefForName.isEmpty()
                                    && rConstraint.msRefForName != rRootCell))
                                continue;
                            std::optional<double> oHeightOfWidth;
                            if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w)
                                oHeightOfWidth = rConstraint.mfFactor;
                            else if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_h)
                                oHeightOfWidth = 1.0 / rConstraint.mfFactor;
                            if (!oHeightOfWidth)
                                continue;
                            const sal_Int32 nOfWidth(
                                static_cast<sal_Int32>(aChildSize.Width * *oHeightOfWidth));
                            if (nOfWidth > 0 && nOfWidth < aChildSize.Height)
                                aChildSize.Height = nOfWidth;
                            break;
                        }
                        // the same where the proportions are parts of the frame
                        if (fHeightOfWidth > 0.0)
                        {
                            const sal_Int32 nOfWidth(
                                static_cast<sal_Int32>(aChildSize.Width * fHeightOfWidth));
                            if (nOfWidth > 0 && nOfWidth < aChildSize.Height)
                                aChildSize.Height = nOfWidth;
                        }
                    }
            }

            awt::Size aConnectorSize = aChildSize;
            aConnectorSize.Width = 1;

            awt::Point aChildPos(0, 0);

            // A run of shapes that holds its proportions is narrower than the room it has along
            // the line, so center there
            if (bHoldsProportions && (nDir == XML_fromL || nDir == XML_fromR))
            {
                const sal_Int32 nGap(static_cast<sal_Int32>(aChildSize.Width * fSpaceWidth));
                const sal_Int32 nRunWidth(aChildSize.Width * nHorizontalShapesCount
                                          + nGap * (nHorizontalShapesCount - 1));
                if (nRunWidth < rShape->getSize().Width)
                    aChildPos.X = (rShape->getSize().Width - nRunWidth) / 2;
            }

            sal_Int32 nLane = 0;
            const sal_Int32 nChildAlign(maMap.count(XML_chAlign) ? maMap.find(XML_chAlign)->second
                                                                 : 0);

            // indent children to show they are descendants, not siblings
            // A straight connector, dim 1D from the middle of one side to the middle of the other,
            // runs in the gap between the root and its branch and needs no lane; a bent one drops
            // down a lane beside the children. A branch whose connectors are all straight keeps
            // its whole width for the children.
            bool bStraightConnectors(false);
            // a branch without any connector has nothing to carry in a lane: Titled_Picture_
            // Accent_List's children stand the whole width of their root
            bool bAnyConnector(false);
            if (mnType == XML_hierChild)
            {
                std::map<OUString, const LayoutNode*> aLayoutNodes;
                gatherLayoutNodes(mrLayoutNode, aLayoutNodes);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    if (pChild->getSubType() != XML_conn)
                        continue;
                    bAnyConnector = true;
                    const auto aNode = aLayoutNodes.find(pChild->getInternalName());
                    const sal_Int32 nRoute(aNode != aLayoutNodes.end() && aNode->second
                                               ? connectorRouteOf(*aNode->second)
                                               : 0);
                    if (nRoute == XML_bend)
                    {
                        bStraightConnectors = false;
                        break;
                    }
                    if (nRoute == XML_stra)
                        bStraightConnectors = true;
                }
            }

            if (mnType == XML_hierChild && nHorizontalShapesCount == 1 && !bStraightConnectors
                && bAnyConnector)
            {
                // The children take the width the layout states for them, and the width left over
                // is the gap that carries the connectors. The child alignment names the side of
                // the branch that it allocates
                const double fChildWidth = readChildOfBranchWidthFactor(rConstraints, rShape);
                nLane = static_cast<sal_Int32>(aChildSize.Width
                                               * (fChildWidth > 0.0 ? 1.0 - fChildWidth : 0.2));
                if (nChildAlign == XML_l)
                    aChildPos.X += nLane;
                else if (nChildAlign != XML_r)
                    aChildPos.X += nLane / 2;
                aChildSize.Width -= nLane;
            }

            sal_Int32 nIdx = 0;
            sal_Int32 nRowHeight = 0;
            for (auto& pChild : rShape->getChildren())
            {
                pChild->setPosition(aChildPos);

                if (mnType == XML_hierChild && pChild->getSubType() == XML_conn)
                {
                    if (nLane > 0 && nChildAlign == XML_l
                        && (nDir == XML_fromT || nDir == XML_fromB))
                    {
                        // The elbow starts where the shape above ends, drops down the middle of
                        // the lane, and turns into the side of the shape it points at
                        const sal_Int32 nGapAbove(
                            static_cast<sal_Int32>(aChildSize.Height * fSpaceHeight));
                        const sal_Int32 nTrunk(nLane / 2);
                        const awt::Point aElbowPos(aChildPos.X - nLane + nTrunk, -nGapAbove);
                        const awt::Size aElbowSize(nLane - nTrunk,
                                                   nGapAbove + aChildPos.Y
                                                       + aChildSize.Height / 2);
                        pChild->setPosition(aElbowPos);
                        pChild->setSize(aElbowSize);
                        pChild->setChildSize(aElbowSize);

                        // A bent connector deines that elbow once its corner sits on its own left
                        // edge, which is where an adjustment of zero puts it
                        pChild->setSubType(XML_bentConnector3);
                        auto& rProperties(*pChild->getCustomShapeProperties());
                        rProperties.setShapePresetType(XML_bentConnector3);
                        if (rProperties.getAdjustmentGuideList().GetCustomShapeGuideValue(
                                u"adj1"_ustr)
                            < 0)
                            rProperties.getAdjustmentGuideList().push_back(
                                { u"adj1"_ustr, u"0"_ustr });
                        continue;
                    }

                    if (nDir == XML_fromL || nDir == XML_fromR)
                    {
                        // The stem of the parent comes down the middle of the branch, and from
                        // there the connector reaches across to the top of the shape that
                        // follows it. What it covers is the room between the two rows and the
                        // way from the middle to that shape.
                        const sal_Int32 nGapAbove(
                            static_cast<sal_Int32>(aChildSize.Height * fSpaceHeight));
                        const sal_Int32 nStem(rShape->getSize().Width / 2);
                        const sal_Int32 nInto(aChildPos.X + aChildSize.Width / 2);
                        const awt::Point aElbowPos(std::min(nStem, nInto), -nGapAbove);
                        const awt::Size aElbowSize(
                            std::max<sal_Int32>(std::abs(nInto - nStem), 1), nGapAbove);
                        pChild->setPosition(aElbowPos);
                        pChild->setSize(aElbowSize);
                        pChild->setChildSize(aElbowSize);
                        continue;
                    }

                    // Connectors should not influence the position of
                    // non-connect shapes.
                    pChild->setSize(aConnectorSize);
                    pChild->setChildSize(aConnectorSize);
                    continue;
                }

                awt::Size aCurrSize = aChildSize;
                aCurrSize.Height *= pChild->getVerticalShapesCount() + (pChild->getVerticalShapesCount() - 1) * fSpaceHeight;
                if (oBranchUnits && nIdx == 1)
                    aCurrSize.Height = static_cast<sal_Int32>(aChildSize.Height * *oBranchUnits);

                // A root beside its branch takes the width its proportions give it, and the
                // branch after it takes whatever room is left, where it lays out its own rows
                // in turn. Sharing the width out evenly would halve the columns at every level
                // down. Where the layout says the root is centred on its children, it stands at
                // the middle of the height of the branch.
                if (mnType == XML_hierRoot && bHoldsProportions
                    && (nDir == XML_fromL || nDir == XML_fromR))
                {
                    if (nIdx > 0)
                        aCurrSize.Width = std::max<sal_Int32>(
                            rShape->getSize().Width - aChildPos.X, aCurrSize.Width);
                    else
                    {
                        const sal_Int32 nHierAlign(maMap.count(XML_hierAlign)
                                                       ? maMap.find(XML_hierAlign)->second
                                                       : 0);
                        if (nHierAlign == XML_lCtrCh || nHierAlign == XML_rCtrCh)
                            pChild->setPosition(awt::Point(
                                aChildPos.X, (rShape->getSize().Height - aCurrSize.Height) / 2));
                    }
                }

                pChild->setSize(aCurrSize);
                pChild->setChildSize(aCurrSize);

                if (nDir == XML_fromT || nDir == XML_fromB)
                    aChildPos.Y += aCurrSize.Height + aChildSize.Height * fSpaceHeight;
                else
                    aChildPos.X += aCurrSize.Width + aCurrSize.Width * fSpaceWidth;

                nRowHeight = std::max(nRowHeight, aCurrSize.Height);

                if (nSecDir == XML_fromT && nIdx % 2 == 1)
                {
                    aChildPos.X = 0;
                    aChildPos.Y += nRowHeight + aChildSize.Height * fSpaceHeight;
                    nRowHeight = 0;
                }

                nIdx++;
            }

            break;
        }

        case XML_lin:
        {
            // spread children evenly across one axis, stretch across second

            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

            const sal_Int32 nDir = maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromL;
            const sal_Int32 nIncX = nDir==XML_fromL ? 1 : (nDir==XML_fromR ? -1 : 0);
            const sal_Int32 nIncY = nDir==XML_fromT ? 1 : (nDir==XML_fromB ? -1 : 0);

            double fCount = rShape->getChildren().size();
            sal_Int32 nConnectorAngle = 0;
            switch (nDir)
            {
            case XML_fromL: nConnectorAngle = 0; break;
            case XML_fromR: nConnectorAngle = 180; break;
            case XML_fromT: nConnectorAngle = 90; break;
            case XML_fromB: nConnectorAngle = 270; break;
            }

            awt::Size aSpaceSize;

            // Find out which constraint is relevant for which (internal) name.
            LayoutPropertyMap aProperties;
            for (const auto& rConstraint : rConstraints)
            {
                if (rConstraint.msForName.isEmpty())
                    continue;

                LayoutProperty& rProperty = aProperties[rConstraint.msForName];
                // These are the plain sizes, a part of the row's own side of the kind stated,
                // whatever the statement refers to, and the extents below read the references
                // properly. One statement is read here as it stands: a part of the shape's own
                // other side, "h refType w refFor des refForName bigChev fact 0.4" on
                // Chevron_List, where the row knows that side, the chevron's width being the
                // whole row. Read as a part of the row's height it made a height the tie below
                // took for a limit, and the chevron came out 0.06 of the row for 0.32.
                const bool bOfItself((rConstraint.mnRefType == XML_w
                                      || rConstraint.mnRefType == XML_h)
                                     && !rConstraint.msForName.isEmpty()
                                     && rConstraint.msRefForName == rConstraint.msForName
                                     && rProperty.count(rConstraint.mnRefType));
                // A side stated against the font is as many millimetres as that part of the
                // font in points, the font of the node it names or of the shape's own:
                // Vertical_Box_List's parent text of "h refType primFontSz fact 0.82" at 18
                // points is 14.76 mm high in its row of that height.
                std::optional<double> oOfFont;
                if (rConstraint.mnRefType == XML_primFontSz
                    && (rConstraint.mnType == XML_w || rConstraint.mnType == XML_h))
                {
                    OUString aGroup;
                    oOfFont = rConstraint.mfFactor
                              * fontOfGroup(rDgm, rConstraints,
                                            rConstraint.msRefForName.isEmpty()
                                                ? rConstraint.msForName
                                                : rConstraint.msRefForName,
                                            aGroup)
                              * 36000.0;
                }
                // a "for des" width in a column across a row is a part of the row's width, the
                // extent the column's need was read against
                const auto aOfRow
                    = const_cast<SmartArtDiagram&>(rDgm).getWidthOfDescendantSizes().find(
                        rShape.get());
                const bool bOfRow(rConstraint.mnFor == XML_des && rConstraint.mnRefType == XML_w
                                  && rConstraint.msRefForName.isEmpty()
                                  && aOfRow
                                         != const_cast<SmartArtDiagram&>(rDgm)
                                                .getWidthOfDescendantSizes()
                                                .end());
                if (rConstraint.mnType == XML_w)
                {
                    rProperty[XML_w] = oOfFont ? *oOfFont
                                               : (bOfItself ? rProperty[rConstraint.mnRefType]
                                                  : bOfRow  ? aOfRow->second
                                                            : rShape->getSize().Width)
                                                     * rConstraint.mfFactor;
                    if (rProperty[XML_w] > rShape->getSize().Width)
                    {
                        rProperty[XML_w] = rShape->getSize().Width;
                    }
                }
                if (rConstraint.mnType == XML_h)
                {
                    rProperty[XML_h] = oOfFont ? *oOfFont
                                               : (bOfItself ? rProperty[rConstraint.mnRefType]
                                                            : rShape->getSize().Height)
                                                     * rConstraint.mfFactor;
                    if (rProperty[XML_h] > rShape->getSize().Height)
                    {
                        rProperty[XML_h] = rShape->getSize().Height;
                    }
                }

                if (rConstraint.mnType == XML_primFontSz && rConstraint.mnFor == XML_des
                    && rConstraint.mnOperator == XML_equ)
                {
                    NamedShapePairs& rDiagramFontHeights = const_cast<SmartArtDiagram&>(rDgm).getDiagramFontHeights();
                    auto it = rDiagramFontHeights.find(rConstraint.msForName);
                    if (it == rDiagramFontHeights.end())
                    {
                        // Start tracking all shapes with this internal name: they'll have the same
                        // font height.
                        rDiagramFontHeights[rConstraint.msForName] = {};
                    }
                }

                // TODO: get values from differently named constraints as well
                if (rConstraint.msForName == "sp" || rConstraint.msForName == "space" || rConstraint.msForName == "sibTrans")
                {
                    if (rConstraint.mnType == XML_w)
                        aSpaceSize.Width = rShape->getSize().Width * rConstraint.mfFactor;
                    if (rConstraint.mnType == XML_h)
                        aSpaceSize.Height = rShape->getSize().Height * rConstraint.mfFactor;
                }
            }

            // Work out the extent every child asks for, so the space can be handed out in the
            // proportions the layout states instead of in equal shares.
            const sal_Int32 nAxisType = nIncX ? XML_w : XML_h;
            const sal_Int32 nParentExtent
                = nIncX ? rShape->getSize().Width : rShape->getSize().Height;
            LinearChildExtents aExtents;
            // below 1.0 where a column across the row needs more room than the row has, and the
            // whole row shrinks by it
            double fUniformFit(1.0);
            // the need of every column across the row whose wishes are shares, by the column's
            // name, and the largest of them
            std::map<const Shape*, double> aShareNeedOfColumn;
            double fLargestShareNeed(0.0);
            if (nIncX || nIncY)
                aExtents.read(rDgm, getLayoutNode(), rShape, rConstraints, nAxisType,
                              nParentExtent,
                              nIncX ? rShape->getSize().Height : rShape->getSize().Width);

            // A line that names the shape it runs to, "dim 1D" with a dstNode, is drawn between
            // shapes and takes no room of its own along the row: Sub-Step_Process's lines from a
            // step's circle to its sub-steps stand in the column of the sub-steps, and asked
            // for the whole of it each, so the column shrank its rows to a sixth. The settle
            // pass draws them, see settleNamedConnectors.
            std::map<OUString, const LayoutNode*> aRowNodes;
            std::map<const Shape*, rtl::Reference<svx::diagram::Point>> aRowPoints;
            if (nIncX || nIncY)
            {
                gatherLayoutNodes(mrLayoutNode, aRowNodes);
                for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                    if (rEntry.second)
                        aRowPoints[rEntry.second.get()] = rEntry.first;
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    if (pChild->getSubType() != XML_conn || aExtents.get(*pChild))
                        continue;
                    const auto aNode = aRowNodes.find(pChild->getInternalName());
                    if (aNode == aRowNodes.end() || !aNode->second)
                        continue;
                    const auto aPoint = aRowPoints.find(pChild.get());
                    const AlgAtom* pLineAlg(algorithmOf(
                        rDgm, *aNode->second,
                        aPoint != aRowPoints.end() ? aPoint->second
                                                   : rtl::Reference<svx::diagram::Point>()));
                    if (!pLineAlg || pLineAlg->getType() != XML_conn
                        || pLineAlg->getNamedParam(XML_dstNode).isEmpty())
                        continue;
                    const auto aDim = pLineAlg->getMap().find(XML_dim);
                    if (aDim == pLineAlg->getMap().end() || aDim->second != XML_1D)
                        continue;
                    aExtents.set(pChild->getInternalName(), 0.0, false);
                }
            }

            // What a child needs along this axis, read from the leaves up, as a part of this
            // row's extent like every wish here, so that the row's fit scales them all by one
            // factor. A row below that may grow along the axis, "rule type=w val=INF" on a lin
            // of the same direction, needs what its own children's wishes add up to, and a
            // column across needs its widest child; a column along the axis with no wish of its
            // own needs the sum of its children. A leaf's own statement as a part of a name of
            // the layout's, "h refType=userH" on a node, where that name is a part of a shape
            // this row knows, "userH for des ptType=node refType=h refFor=des refForName=txOne",
            // is its wish. Table_Hierarchy: columns of a node over a row of its children, every
            // node and space a part of the root's width; the drawing divides the top row by how
            // many leaves each column holds, 3.168, 2.084, 1 and 2.084 widths and the spaces
            // between, and makes the row below a node as high as that node, userH, so the
            // column of a node, its gap and its children's row scales by 0.48.
            // rbGrown is set where a row below grows, rbFromBelow where the need came from the
            // children at all.
            std::function<std::optional<double>(const ShapePtr&, const LinearChildExtents&, bool&,
                                                bool&)>
                aNeed = [&](const ShapePtr& pOne, const LinearChildExtents& rOfRow, bool& rbGrown,
                            bool& rbFromBelow) -> std::optional<double> {
                const auto aNode = aRowNodes.find(pOne->getInternalName());
                const LayoutNode* pNode(aNode != aRowNodes.end() ? aNode->second : nullptr);
                if (pNode)
                {
                    std::vector<const AlgAtom*> aAlgorithms;
                    gatherOwnAlgorithms(*pNode, aAlgorithms);
                    const AlgAtom* pLin(nullptr);
                    for (const AlgAtom* pAlg : aAlgorithms)
                        if (pAlg->getType() == XML_lin)
                            pLin = pAlg;
                    if (pLin && !pOne->getChildren().empty())
                    {
                        const auto aDir = pLin->getMap().find(XML_linDir);
                        const sal_Int32 nBelowDir(aDir != pLin->getMap().end() ? aDir->second
                                                                               : XML_fromL);
                        const bool bBelowAlong(nBelowDir == XML_fromL || nBelowDir == XML_fromR);
                        std::vector<Constraint> aBelowConstraints(rConstraints);
                        const std::vector<Constraint> aOwn(collectDirectConstraints(*pNode));
                        aBelowConstraints.insert(aBelowConstraints.end(), aOwn.begin(),
                                                 aOwn.end());
                        LinearChildExtents aBelow;
                        aBelow.read(rDgm, *pNode, pOne, aBelowConstraints, nAxisType,
                                    nParentExtent,
                                    nIncX ? rShape->getSize().Height : rShape->getSize().Width);
                        const bool bGrows(bBelowAlong == bool(nIncX)
                                          && ownRuleGrows(*pNode, nAxisType));
                        if (bBelowAlong == bool(nIncX) && (bGrows || !rOfRow.get(*pOne)))
                        {
                            std::optional<double> oSum;
                            for (const ShapePtr& pInside : pOne->getChildren())
                            {
                                const std::optional<double> oInside(
                                    aNeed(pInside, aBelow, rbGrown, rbFromBelow));
                                if (oInside)
                                    oSum = oSum.value_or(0.0) + *oInside;
                            }
                            if (bGrows)
                                rbGrown = true;
                            if (oSum)
                            {
                                rbFromBelow = true;
                                return oSum;
                            }
                        }
                        if (bBelowAlong != bool(nIncX))
                        {
                            std::optional<double> oWidest;
                            for (const ShapePtr& pInside : pOne->getChildren())
                            {
                                const std::optional<double> oInside(
                                    aNeed(pInside, aBelow, rbGrown, rbFromBelow));
                                if (oInside && (!oWidest || *oInside > *oWidest))
                                    oWidest = oInside;
                            }
                            if (oWidest)
                            {
                                rbFromBelow = true;
                                return oWidest;
                            }
                        }
                    }
                }
                const std::optional<double> oWish(rOfRow.get(*pOne));
                if (oWish || !pNode)
                    return oWish;
                for (const Constraint& rOwn : collectDirectConstraints(*pNode))
                {
                    if (rOwn.mnFor != XML_self || rOwn.mnType != nAxisType
                        || !isUserVariable(rOwn.mnRefType))
                        continue;
                    for (const Constraint& rName : rConstraints)
                    {
                        if (rName.mnType != rOwn.mnRefType || rName.mnRefType != nAxisType
                            || rName.msRefForName.isEmpty())
                            continue;
                        const std::optional<double> oOf(rOfRow.stated(rName.msRefForName));
                        if (!oOf)
                            continue;
                        return *oOf * (rName.mfFactor != 0.0 ? rName.mfFactor : 1.0)
                               * (rOwn.mfFactor != 0.0 ? rOwn.mfFactor : 1.0);
                    }
                }
                return std::nullopt;
            };
            if (nIncX || nIncY)
            {
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    bool bGrown(false);
                    bool bFromBelow(false);
                    const std::optional<double> oNeed(aNeed(pChild, aExtents, bGrown, bFromBelow));
                    if (bGrown && oNeed && *oNeed > 0.0)
                        aExtents.setForShape(*pChild, *oNeed);
                }
            }

            // A text node whose box may grow along the column, "rule type=h val=INF" on a tx
            // node, is as high as its text and the margins over and under it, at least the
            // part of the font it is stated with, and the font is the largest the layout allows
            // that lets the column hold every such node: the layout states the size, primFontSz
            // 65, and the least it may shrink to, "rule type=primFontSz val=5", and every size
            // stated as a part of primFontSz follows the font. Vertical_Bullet_List's headers
            // are 0.52 of the font high and grow to one line of text with margins of 0.3 of the
            // font, its child texts 0.46 of the font and grow to their two or three lines, and
            // at 27 points the column holds the seven of them and their spacers of 0.08 to 97
            // percent of the height, at 28 it does not; the drawing has 27. A row below the
            // column needs its highest child, a column below the sum of its children.
            if (nIncY && !rShape->getChildren().empty() && nParentExtent > 0)
            {
                const double fPoint(12700.0);
                const double fMillimetre(36000.0);
                std::map<OUString, double>& rFitted(
                    const_cast<SmartArtDiagram&>(rDgm).getFittedFontSizes());
                // the least the font may shrink to, the rule's value
                double fLeastFont(5.0);
                if (rDgm.getLayout() && rDgm.getLayout()->getNode())
                {
                    std::function<void(const LayoutAtom&)> aLeast = [&](const LayoutAtom& rAtom) {
                        for (const LayoutAtomPtr& pBelow : rAtom.getChildren())
                        {
                            if (const RuleAtom* pRule = dynamic_cast<const RuleAtom*>(pBelow.get()))
                            {
                                if (pRule->getRule().mnType == XML_primFontSz
                                    && std::isfinite(pRule->getRule().mfValue)
                                    && pRule->getRule().mfValue > 0.0)
                                    fLeastFont = pRule->getRule().mfValue;
                            }
                            else
                                aLeast(*pBelow);
                        }
                    };
                    aLeast(*rDgm.getLayout()->getNode());
                }
                // what a shape needs along the column at a font size, as a part of this
                // column's extent, and whether anything below it is text-driven
                bool bAnyTextDriven(false);
                // false once a text node's widest word is wider than its box at the font tried
                bool bWordsFit(true);
                // the width a row in the column gives a child of its own, from its stated part
                std::map<const Shape*, sal_Int64> aWidthInRow;
                std::vector<ShapePtr> aTextNodes;
                // the groups a text of which has a word wider than its box at the font tried
                std::set<OUString> aGroupsWordsDidNotFit;
                // a font given as 0 means the font of the node's own group on this pass
                std::function<std::optional<double>(const ShapePtr&, double, bool)> aNeedAt
                    = [&](const ShapePtr& pOne, double fFontGiven,
                          bool bAlongColumn) -> std::optional<double> {
                    const auto aNode = aRowNodes.find(pOne->getInternalName());
                    const LayoutNode* pNode(aNode != aRowNodes.end() ? aNode->second : nullptr);
                    OUString aGroup;
                    const double fFont(fFontGiven > 0.0 ? fFontGiven
                                                        : fontOfGroup(rDgm, rConstraints,
                                                                      pOne->getInternalName(),
                                                                      aGroup));
                    // a text of a size of its own is measured at that size with its margins,
                    // what the layout states against the font stays at the group's
                    const std::optional<double> oOwnSize(ownTextSizeOf(rDgm, pOne));
                    // the part of the font the shape is stated with along the column, as many
                    // millimetres as the part of the font in points: Vertical_Box_List's parent
                    // text of "h refType primFontSz fact 0.82" at 18 points is 14.76 mm high.
                    // The font is the one of the node the statement refers to, Bracket_List's
                    // spV and brace "refForName parTx", of the shape's own where it names none.
                    std::optional<double> oPartOfFont;
                    for (const Constraint& rConstraint : rConstraints)
                        if (rConstraint.mnType == nAxisType
                            && rConstraint.mnRefType == XML_primFontSz
                            && rConstraint.msForName == pOne->getInternalName()
                            && rConstraint.mfFactor != 0.0 && rConstraint.mnOperator == XML_none)
                        {
                            OUString aRefGroup;
                            const double fRefFont(
                                fFontGiven > 0.0 ? fFontGiven
                                                 : fontOfGroup(rDgm, rConstraints,
                                                               rConstraint.msRefForName.isEmpty()
                                                                   ? pOne->getInternalName()
                                                                   : rConstraint.msRefForName,
                                                               aRefGroup));
                            oPartOfFont
                                = rConstraint.mfFactor * fRefFont * fMillimetre / nParentExtent;
                        }
                    if (!pNode)
                        return oPartOfFont;
                    std::vector<const AlgAtom*> aAlgorithms;
                    gatherOwnAlgorithms(*pNode, aAlgorithms);
                    const AlgAtom* pAlg(aAlgorithms.empty() ? nullptr : aAlgorithms.front());
                    if (pAlg && pAlg->getType() == XML_tx && ownRuleGrows(*pNode, nAxisType))
                    {
                        // a node without text is the part of the font it is stated with, or just
                        // its margins
                        const std::vector<OUString> aParagraphs(textParagraphsOf(rDgm, pOne));
                        {
                            bAnyTextDriven = true;
                            if (std::find(aTextNodes.begin(), aTextNodes.end(), pOne)
                                == aTextNodes.end())
                                aTextNodes.push_back(pOne);
                            if (aParagraphs.empty() && oPartOfFont)
                                return oPartOfFont;
                            // the margins, stated for the node by the row or by the node itself
                            // as parts of the font, or as lengths
                            double fOverUnder(0.0), fSides(0.0);
                            sal_Int64 nFixed(0);
                            const auto aInRow = aWidthInRow.find(pOne.get());
                            sal_Int64 nWidth(aInRow != aWidthInRow.end()
                                                 ? aInRow->second
                                                 : rShape->getSize().Width);
                            const auto aMargins = [&](const Constraint& rConstraint, bool bOwn) {
                                if (!bOwn && rConstraint.msForName != pOne->getInternalName())
                                    return;
                                if (bOwn && !rConstraint.msForName.isEmpty())
                                    return;
                                if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_w
                                    && rConstraint.msRefForName.isEmpty()
                                    && rConstraint.mfFactor > 0.0 && !bOwn)
                                    nWidth = static_cast<sal_Int64>(rShape->getSize().Width
                                                                    * rConstraint.mfFactor);
                                const bool bOverUnder(rConstraint.mnType == XML_tMarg
                                                      || rConstraint.mnType == XML_bMarg);
                                const bool bSide(rConstraint.mnType == XML_lMarg
                                                 || rConstraint.mnType == XML_rMarg);
                                if (!bOverUnder && !bSide)
                                    return;
                                if (rConstraint.mnRefType == XML_primFontSz)
                                    (bOverUnder ? fOverUnder : fSides) += rConstraint.mfFactor;
                                else if (rConstraint.mnRefType == XML_w && bSide)
                                    nWidth -= static_cast<sal_Int64>(nWidth * rConstraint.mfFactor);
                                else if (rConstraint.mnRefType == XML_none
                                         && rConstraint.mfValue > 0.0 && bOverUnder)
                                    nFixed += static_cast<sal_Int64>(o3tl::convert(
                                        rConstraint.mfValue, o3tl::Length::mm, o3tl::Length::emu));
                            };
                            for (const Constraint& rConstraint : rConstraints)
                                aMargins(rConstraint, false);
                            // the node's own statements, the branch of a choose decided by the
                            // node's point, one statement per margin, and a margin stated as
                            // another margin, "bMarg refType tMarg", is that one's
                            std::vector<Constraint> aOwn;
                            {
                                const auto aPoint = aRowPoints.find(pOne.get());
                                const rtl::Reference<svx::diagram::Point> xPoint(
                                    aPoint != aRowPoints.end()
                                        ? aPoint->second
                                        : rtl::Reference<svx::diagram::Point>());
                                gatherDecidedConstraints(rDgm, *pNode, xPoint, aOwn);
                            }
                            std::set<sal_Int32> aSeen;
                            for (const Constraint& rConstraint : aOwn)
                            {
                                if (!rConstraint.msForName.isEmpty()
                                    || aSeen.count(rConstraint.mnType))
                                    continue;
                                aSeen.insert(rConstraint.mnType);
                                if ((rConstraint.mnRefType == XML_tMarg
                                     || rConstraint.mnRefType == XML_bMarg)
                                    && (rConstraint.mnType == XML_tMarg
                                        || rConstraint.mnType == XML_bMarg))
                                {
                                    for (const Constraint& rOther : aOwn)
                                        if (rOther.mnType == rConstraint.mnRefType
                                            && rOther.mnRefType == XML_primFontSz)
                                        {
                                            fOverUnder += rOther.mfFactor
                                                          * (rConstraint.mfFactor != 0.0
                                                                 ? rConstraint.mfFactor
                                                                 : 1.0);
                                            break;
                                        }
                                    continue;
                                }
                                aMargins(rConstraint, true);
                            }
                            fSides += unstatedMarginPart(rConstraints, aOwn,
                                                         pOne->getInternalName(), false);
                            fOverUnder += unstatedMarginPart(rConstraints, aOwn,
                                                             pOne->getInternalName(), true);
                            // the text of a deeper level is smaller, each paragraph by the
                            // depth of the node it comes from under the group's level; the
                            // margins stated as parts of the font follow the first level's
                            const std::vector<sal_Int32> aLevels(
                                textParagraphLevelsOf(rDgm, pOne, aParagraphs.size()));
                            const double fSecondPart(
                                secondFontPartOf(rConstraints, *pNode, pOne->getInternalName()));
                            const double fOwnFont(oOwnSize ? *oOwnSize : fFont);
                            std::vector<double> aFontOfParagraph;
                            for (size_t n = 0; n < aParagraphs.size(); ++n)
                                aFontOfParagraph.push_back(
                                    oOwnSize ? fOwnFont
                                             : fontOfLevel(fFont, aLevels[n], fSecondPart));
                            const double fTextFont(aFontOfParagraph.empty()
                                                       ? fOwnFont
                                                       : aFontOfParagraph.front());
                            nWidth -= static_cast<sal_Int64>(fSides * fOwnFont * fPoint);
                            // the words stand in the width the shape leaves its text
                            sal_Int64 nInset(0);
                            const sal_Int64 nText(
                                aParagraphs.empty()
                                    ? 0
                                    : measureTextHeightInShape(
                                          pOne, aParagraphs, std::max<sal_Int64>(nWidth, 1),
                                          nFixed
                                              + static_cast<sal_Int64>(fOverUnder * fOwnFont
                                                                       * fPoint),
                                          fTextFont, aFontOfParagraph, aLevels,
                                          spaceAfterPartsOf(*pNode), rDgm.getTextFont(), &nInset));
                            if (!aParagraphs.empty()
                                && measureWidestWord(aParagraphs, fTextFont,
                                                     rDgm.getTextFont(), aFontOfParagraph,
                                                     aLevels)
                                       > nWidth - 2 * nInset)
                            {
                                bWordsFit = false;
                                if (!aGroup.isEmpty() && !oOwnSize)
                                    aGroupsWordsDidNotFit.insert(aGroup);
                            }
                            const sal_Int64 nOverUnder(
                                static_cast<sal_Int64>(fOverUnder * fOwnFont * fPoint));
                            const double fNeed(static_cast<double>(nText + nFixed + nOverUnder)
                                               / nParentExtent);
                            return std::max(fNeed, oPartOfFont.value_or(0.0));
                        }
                    }
                    if (pAlg && pAlg->getType() == XML_lin && !pOne->getChildren().empty()
                        && !oPartOfFont)
                    {
                        const auto aDir = pAlg->getMap().find(XML_linDir);
                        const sal_Int32 nBelowDir(aDir != pAlg->getMap().end() ? aDir->second
                                                                               : XML_fromL);
                        const bool bBelowAlong(nBelowDir == XML_fromT || nBelowDir == XML_fromB);
                        std::optional<double> oBelow;
                        std::vector<Constraint> aRowOwn;
                        if (!bBelowAlong)
                        {
                            const auto aPoint = aRowPoints.find(pOne.get());
                            gatherDecidedConstraints(rDgm, *pNode,
                                                     aPoint != aRowPoints.end()
                                                         ? aPoint->second
                                                         : rtl::Reference<svx::diagram::Point>(),
                                                     aRowOwn);
                        }
                        for (const ShapePtr& pInside : pOne->getChildren())
                        {
                            for (const Constraint& rShare : aRowOwn)
                                if (rShare.mnType == XML_w && rShare.mnRefType == XML_w
                                    && rShare.mnFor == XML_ch
                                    && rShare.msForName == pInside->getInternalName()
                                    && rShare.msRefForName.isEmpty() && rShare.mfFactor > 0.0
                                    && rShare.mnOperator == XML_none)
                                    aWidthInRow[pInside.get()] = static_cast<sal_Int64>(
                                        rShape->getSize().Width * rShare.mfFactor);
                            const std::optional<double> oInside(
                                aNeedAt(pInside, fFontGiven, bBelowAlong));
                            if (!oInside)
                                continue;
                            oBelow = bBelowAlong ? oBelow.value_or(0.0) + *oInside
                                                 : std::max(oBelow.value_or(0.0), *oInside);
                        }
                        if (oBelow)
                            return oBelow;
                    }
                    // A composite whose texts grow with their text is as high as they need it,
                    // as the pass before found: Numbered_List's composites are their turned
                    // descriptions' heights, 5176 for three paragraphs and 1726 for one.
                    if (pAlg && pAlg->getType() == XML_composite && bAlongColumn && !oPartOfFont)
                    {
                        SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                            if (rEntry.second == pOne && rEntry.first.is())
                            {
                                const auto aKept
                                    = rMutable.getContentHeights().find(rEntry.first->msModelId);
                                if (aKept == rMutable.getContentHeights().end())
                                {
                                    rMutable.getContentHeightMissing() = true;
                                    break;
                                }
                                bAnyTextDriven = true;
                                return static_cast<double>(aKept->second) / nParentExtent;
                            }
                    }
                    if (oPartOfFont)
                        return oPartOfFont;
                    return bAlongColumn ? aExtents.get(*pOne) : std::nullopt;
                };
                // the first pass finds the text nodes and the stated font
                for (const ShapePtr& pChild : rShape->getChildren())
                    aNeedAt(pChild, 65.0, true);
                if (bAnyTextDriven)
                {
                    // a column given far more room than the frame, Vertical_Header_List's lin
                    // of 100 heights, fits the texts to the frame
                    double fRoom(1.0);
                    if (rDgm.getLayout() && rDgm.getLayout()->getNode()
                        && rDgm.getLayout()->getNode()->getExistingShape())
                    {
                        const sal_Int32 nFrame(
                            rDgm.getLayout()->getNode()->getExistingShape()->getSize().Height);
                        if (nFrame > 0 && nFrame < nParentExtent)
                            fRoom = static_cast<double>(nFrame) / nParentExtent;
                    }
                    // The font of this pass, by the group each text node is of, the stated
                    // size on the first; the column holds its texts at it or it tells the
                    // Diagram so, and the next pass runs a point smaller for the groups that
                    // did not fit: Continuous_Arrow_Process's column is far from full, and its
                    // Thirteen fits its box of 114 points at 27 points and not above, so the
                    // passes go down to 27.
                    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
                    rMutable.getLeastFont() = std::min(rMutable.getLeastFont(), fLeastFont);
                    const double fFont(0.0);
                    {
                        double fTotal(0.0);
                        bWordsFit = true;
                        aGroupsWordsDidNotFit.clear();
                        // a child of a negative size overlaps its neighbours and gives the
                        // column that much back: Vertical_Box_List's space of -0.41 of the font
                        // under each parent text
                        for (const ShapePtr& pChild : rShape->getChildren())
                            if (const std::optional<double> oOne = aNeedAt(pChild, fFont, true))
                                fTotal += *oOne;
                        // a column too full shrinks all its groups, a word too wide its own
                        if (fTotal > fRoom + 1e-6)
                            for (const ShapePtr& pText : aTextNodes)
                                rMutable.getGroupsThatDidNotFit().insert(
                                    textGroupOf(rDgm, rConstraints, pText));
                        for (const OUString& rGroup : aGroupsWordsDidNotFit)
                            rMutable.getGroupsThatDidNotFit().insert(rGroup);
                        // A shape bound both ways to a part of a text node's height, Continuous_
                        // Arrow_Process's arrow of twice the step text, has to stand in the
                        // frame as well, so the text shrinks until it does.
                        if (rDgm.getLayout() && rDgm.getLayout()->getNode()
                            && rDgm.getLayout()->getNode()->getExistingShape())
                        {
                            const sal_Int32 nFrame(
                                rDgm.getLayout()->getNode()->getExistingShape()->getSize().Height);
                            std::vector<Constraint> aBounds;
                            std::function<void(const LayoutAtom&)> aGather
                                = [&](const LayoutAtom& rAtom) {
                                      for (const LayoutAtomPtr& pBelow : rAtom.getChildren())
                                      {
                                          const ConstraintAtom* pConstraint(
                                              dynamic_cast<const ConstraintAtom*>(pBelow.get()));
                                          if (pConstraint)
                                          {
                                              const Constraint& rOne(pConstraint->getConstraint());
                                              if (rOne.mnOperator == XML_gte)
                                                  aBounds.push_back(rOne);
                                          }
                                          else
                                              aGather(*pBelow);
                                      }
                                  };
                            aGather(*rDgm.getLayout()->getNode());
                            for (const ShapePtr& pText : aTextNodes)
                            {
                                const std::optional<double> oNeed(aNeedAt(pText, fFont, true));
                                if (!oNeed || nFrame <= 0)
                                    continue;
                                for (const Constraint& rBound : aBounds)
                                    if (rBound.mnType == XML_h && rBound.mnRefType == XML_h
                                        && rBound.msRefForName == pText->getInternalName()
                                        && rBound.mfFactor > 0.0
                                        && *oNeed * nParentExtent * rBound.mfFactor > nFrame)
                                        rMutable.getGroupsThatDidNotFit().insert(
                                            textGroupOf(rDgm, rConstraints, pText));
                            }
                        }
                    }
                    for (const ShapePtr& pChild : rShape->getChildren())
                        if (const std::optional<double> oOne = aNeedAt(pChild, fFont, true))
                            if (oOne != aExtents.get(*pChild))
                                aExtents.setForShape(*pChild, *oOne);
                    for (const ShapePtr& pText : aTextNodes)
                    {
                        OUString aGroup;
                        const double fGroupFont(
                            textFontOf(rDgm, rConstraints, pText, aGroup));
                        const auto aTextNode = aRowNodes.find(pText->getInternalName());
                        const double fSecondPart(
                            aTextNode != aRowNodes.end() && aTextNode->second
                                ? secondFontPartOf(rConstraints, *aTextNode->second,
                                                   pText->getInternalName())
                                : 0.78);
                        if (rMutable.getGroupsThatShrank().count(aGroup))
                            rFitted[pText->getInternalName()] = fontOfLevel(
                                fGroupFont, textParagraphLevelsOf(rDgm, pText, 1).front(),
                                fSecondPart);
                    }
                    aExtents.refreshOverlap(rShape->getChildren());
                }
            }

            // In a row, lin fromL, a text node that grows across with its text, "rule type=w",
            // is as wide as its widest line and its side margins, Accented_Picture's child
            // texts in their wrapper beside the circles, 4939 for a wrapper of twice that.
            if (nIncX && !rShape->getChildren().empty() && nParentExtent > 0)
            {
                bool bAnyWidth(false);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const auto aNode = aRowNodes.find(pChild->getInternalName());
                    if (aNode == aRowNodes.end() || !aNode->second
                        || pChild->getDiagramRotation() != 0 || pChild->getRotation() != 0)
                        continue;
                    const auto aPoint = aRowPoints.find(pChild.get());
                    const std::optional<sal_Int64> oNeed(textDrivenWidth(
                        rDgm, rConstraints, *aNode->second,
                        aPoint != aRowPoints.end() ? aPoint->second
                                                   : rtl::Reference<svx::diagram::Point>(),
                        pChild));
                    if (!oNeed)
                        continue;
                    if (*oNeed > nParentExtent)
                        const_cast<SmartArtDiagram&>(rDgm).getGroupsThatDidNotFit().insert(
                            textGroupOf(rDgm, rConstraints, pChild));
                    // A text alone in its row, Circular_Picture_Callout's and Accented_
                    // Picture's texts in a wrapper that aligns them to a side, is as wide as
                    // its words. Among siblings the text only grows a box past the width the
                    // row states for it, and a node the row gives an equal share, Continuous_
                    // Block_Process's, keeps it.
                    const double fNeed(std::min(1.0, static_cast<double>(*oNeed) / nParentExtent));
                    const std::optional<double> oStated(aExtents.get(*pChild));
                    const bool bAlone(rShape->getChildren().size() == 1);
                    if (!bAlone && (!oStated || *oStated >= fNeed))
                        continue;
                    aExtents.setForShape(*pChild, fNeed);
                    bAnyWidth = true;
                }
                if (bAnyWidth)
                    aExtents.refreshOverlap(rShape->getChildren());
            }

            // In a column, lin fromT, a text node that grows across with its text, "rule type=w
            // val=INF", is as wide as its widest line and its side margins, at least the width
            // stated for it and at most the column's: Varying_Width_List's texts of a stated
            // 20 mm are 45, 57, 34 and 50 thirty-seconds of that, each its own words wide. The
            // width is kept by the shape, the texts of a column sharing their name.
            std::map<const Shape*, sal_Int32> aTextWidthAcross;
            if (nIncY && !rShape->getChildren().empty())
            {
                const sal_Int64 nRoom(rShape->getSize().Width);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const auto aNode = aRowNodes.find(pChild->getInternalName());
                    if (aNode == aRowNodes.end() || !aNode->second
                        || pChild->getDiagramRotation() != 0 || pChild->getRotation() != 0
                        || !ownRuleGrows(*aNode->second, XML_w))
                        continue;
                    const auto aPoint = aRowPoints.find(pChild.get());
                    const std::optional<sal_Int64> oNeed(textDrivenWidth(
                        rDgm, rConstraints, *aNode->second,
                        aPoint != aRowPoints.end() ? aPoint->second
                                                   : rtl::Reference<svx::diagram::Point>(),
                        pChild));
                    if (!oNeed)
                        continue;
                    if (*oNeed > nRoom)
                        const_cast<SmartArtDiagram&>(rDgm).getGroupsThatDidNotFit().insert(
                            textGroupOf(rDgm, rConstraints, pChild));
                    // the width stated for the text as a length, "w for ch forName text val
                    // 20", in millimetres
                    sal_Int64 nStated(0);
                    for (const Constraint& rConstraint : rConstraints)
                        if (rConstraint.mnType == XML_w && rConstraint.mnRefType == XML_none
                            && rConstraint.msForName == pChild->getInternalName()
                            && rConstraint.mfValue > 0.0)
                            nStated = static_cast<sal_Int64>(o3tl::convert(
                                rConstraint.mfValue, o3tl::Length::mm, o3tl::Length::emu));
                    const sal_Int64 nWidth(std::min(nRoom, std::max(nStated, *oNeed)));
                    aTextWidthAcross[pChild.get()] = static_cast<sal_Int32>(nWidth);
                    // The text's height is its share of the column, and it holds its text at
                    // that width or its group is a point smaller on the next pass.
                    const std::optional<double> oAlong(aExtents.get(*pChild));
                    if (!oAlong || nParentExtent <= 0)
                        continue;
                    // the shares are scaled down together where they add up past the column
                    double fTotal(0.0);
                    for (const ShapePtr& pOther : rShape->getChildren())
                        fTotal += aExtents.get(*pOther).value_or(0.0);
                    const sal_Int64 nSlot(
                        static_cast<sal_Int64>(*oAlong / std::max(1.0, fTotal) * nParentExtent));
                    bool bWordsFit(true);
                    const std::optional<sal_Int64> oHeightNeed(textDrivenHeight(
                        rDgm, rConstraints, *aNode->second,
                        aPoint != aRowPoints.end() ? aPoint->second
                                                   : rtl::Reference<svx::diagram::Point>(),
                        pChild, nWidth, bWordsFit, /*bGrowingOnly*/ false));
                    if (oHeightNeed
                        && (*oHeightNeed > nSlot || !bWordsFit))
                        const_cast<SmartArtDiagram&>(rDgm).getGroupsThatDidNotFit().insert(
                            textGroupOf(rDgm, rConstraints, pChild));
                }
            }

            // the height a row grew a text to, kept by the shape, the texts of a row sharing
            // their name, Bar_List's bars
            std::map<const Shape*, sal_Int32> aTextHeightAcross;
            // the height stated for a name before the texts of that name wrote theirs
            std::map<OUString, sal_Int64> aStatedHeightOf;
            // the children grown from their own proportions by a rule's factor
            std::set<const Shape*> aGrownByFactor;
            // In a row, lin fromL, a text node that grows with its text, "rule type=h val=INF",
            // is as high as its text at the Diagram's font and the margins over and under it,
            // at least the part of the font it is stated with, and what is bound to it follows:
            // Bracket_List's child text of three lines is 4991 high beside a parent text of
            // one line, 1815, and the brace between them takes the higher of the two.
            if (nIncX && !rShape->getChildren().empty())
            {
                SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
                const double fPoint(12700.0);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const auto aNode = aRowNodes.find(pChild->getInternalName());
                    if (aNode == aRowNodes.end() || !aNode->second)
                        continue;
                    std::vector<const AlgAtom*> aAlgorithms;
                    gatherOwnAlgorithms(*aNode->second, aAlgorithms);
                    if (aAlgorithms.empty() || aAlgorithms.front()->getType() != XML_tx)
                        continue;
                    // a text child whose height the row fixes holds its text at the font of
                    // the pass, or its group is a point smaller on the next
                    const bool bGrows(ownRuleGrows(*aNode->second, XML_h));
                    const std::vector<OUString> aParagraphs(textParagraphsOf(rDgm, pChild));
                    OUString aGroup;
                    const double fFont(
                        textFontOf(rDgm, rConstraints, pChild, aGroup));
                    const std::vector<sal_Int32> aLevels(
                        textParagraphLevelsOf(rDgm, pChild, aParagraphs.size()));
                    const double fSecondPart(
                        secondFontPartOf(rConstraints, *aNode->second, pChild->getInternalName()));
                    std::vector<double> aFontOfParagraph;
                    for (size_t n = 0; n < aParagraphs.size(); ++n)
                        aFontOfParagraph.push_back(fontOfLevel(fFont, aLevels[n], fSecondPart));
                    const double fTextFont(aFontOfParagraph.empty() ? fFont
                                                                    : aFontOfParagraph.front());
                    // the width the row gives the child, and the margins stated for it
                    // the shares are scaled down together where they add up past the row
                    sal_Int64 nWidth(rShape->getSize().Width);
                    if (const std::optional<double> oAlong = aExtents.get(*pChild))
                    {
                        double fTotal(0.0);
                        for (const ShapePtr& pOther : rShape->getChildren())
                            fTotal += aExtents.get(*pOther).value_or(0.0);
                        nWidth = static_cast<sal_Int64>(*oAlong / std::max(1.0, fTotal)
                                                        * rShape->getSize().Width);
                    }
                    double fOverUnder(0.0), fSides(0.0);
                    std::optional<double> oPartOfFont;
                    for (const Constraint& rConstraint : rConstraints)
                    {
                        if (rConstraint.msForName != pChild->getInternalName())
                            continue;
                        if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_primFontSz
                            && rConstraint.mfFactor > 0.0 && rConstraint.mnOperator == XML_none)
                            oPartOfFont = rConstraint.mfFactor;
                        if (rConstraint.mnRefType != XML_primFontSz)
                            continue;
                        if (rConstraint.mnType == XML_tMarg || rConstraint.mnType == XML_bMarg)
                            fOverUnder += rConstraint.mfFactor;
                        else if (rConstraint.mnType == XML_lMarg || rConstraint.mnType == XML_rMarg)
                            fSides += rConstraint.mfFactor;
                    }
                    std::vector<Constraint> aOwn;
                    {
                        const auto aPoint = aRowPoints.find(pChild.get());
                        gatherDecidedConstraints(rDgm, *aNode->second,
                                                 aPoint != aRowPoints.end()
                                                     ? aPoint->second
                                                     : rtl::Reference<svx::diagram::Point>(),
                                                 aOwn);
                    }
                    // the node's own height as a part of its width, "h refType w fact 0.6" on
                    // Basic_Process's nodes, is the least it is, the text may only grow it
                    sal_Int64 nOwnHeight(0);
                    std::set<sal_Int32> aSeen;
                    for (const Constraint& rConstraint : aOwn)
                    {
                        if (!rConstraint.msForName.isEmpty() || aSeen.count(rConstraint.mnType))
                            continue;
                        aSeen.insert(rConstraint.mnType);
                        if (rConstraint.mnType == XML_h && rConstraint.mnRefType == XML_w
                            && rConstraint.mfFactor > 0.0 && rConstraint.mnOperator == XML_none)
                            nOwnHeight = static_cast<sal_Int64>(nWidth * rConstraint.mfFactor);
                        const bool bOverUnder(rConstraint.mnType == XML_tMarg
                                              || rConstraint.mnType == XML_bMarg);
                        const bool bSide(rConstraint.mnType == XML_lMarg
                                         || rConstraint.mnType == XML_rMarg);
                        if (!bOverUnder && !bSide)
                            continue;
                        if (rConstraint.mnRefType == XML_primFontSz)
                            (bOverUnder ? fOverUnder : fSides) += rConstraint.mfFactor;
                        else if (rConstraint.mnRefType == XML_tMarg
                                 || rConstraint.mnRefType == XML_bMarg)
                            for (const Constraint& rOther : aOwn)
                                if (rOther.mnType == rConstraint.mnRefType
                                    && rOther.mnRefType == XML_primFontSz)
                                {
                                    fOverUnder += rOther.mfFactor;
                                    break;
                                }
                    }
                    fSides += unstatedMarginPart(rConstraints, aOwn, pChild->getInternalName(),
                                                 false);
                    fOverUnder += unstatedMarginPart(rConstraints, aOwn,
                                                     pChild->getInternalName(), true);
                    nWidth -= static_cast<sal_Int64>(fSides * fFont * fPoint);
                    // the words stand in the width the shape leaves its text
                    sal_Int64 nInset(0);
                    const sal_Int64 nText(
                        aParagraphs.empty()
                            ? 0
                            : measureTextHeightInShape(
                                  pChild, aParagraphs, std::max<sal_Int64>(nWidth, 1),
                                  static_cast<sal_Int64>(fOverUnder * fFont * fPoint), fTextFont,
                                  aFontOfParagraph, aLevels,
                                  spaceAfterPartsOf(*aNode->second), rDgm.getTextFont(), &nInset));
                    if (!aParagraphs.empty()
                        && measureWidestWord(aParagraphs, fTextFont, rDgm.getTextFont(),
                                             aFontOfParagraph, aLevels)
                               > nWidth - 2 * nInset)
                        rMutable.getGroupsThatDidNotFit().insert(aGroup);
                    const sal_Int64 nOverUnder(static_cast<sal_Int64>(fOverUnder * fFont * fPoint));
                    // a height stated as a part of the font is as many millimetres
                    const sal_Int64 nPartOfFont(
                        static_cast<sal_Int64>(oPartOfFont.value_or(0.0) * fFont * 36000.0));
                    sal_Int64 nNeed(std::max<sal_Int64>(nText + nOverUnder, nPartOfFont));
                    if (!bGrows)
                    {
                        if (!aParagraphs.empty() && nText + nOverUnder > rShape->getSize().Height)
                            rMutable.getGroupsThatDidNotFit().insert(aGroup);
                        continue;
                    }
                    // a node with a height of its own width grows by the rule's factor at most
                    // ("rule type=h fact=1.5") once its font is down to the rule's floor before
                    // it; a text higher than the node, or than the grown node, makes its group a
                    // point smaller on the next pass. Nodes held equal ("h for ch ptType node op
                    // equ") are as high as the highest
                    if (nOwnHeight > 0)
                    {
                        const std::optional<std::pair<double, double>> oFactor(
                            ownRuleFactorOf(*aNode->second, XML_h));
                        if (!oFactor || nNeed <= nOwnHeight)
                            continue;
                        const sal_Int64 nMost(
                            static_cast<sal_Int64>(nOwnHeight * oFactor->first));
                        if (fFont > oFactor->second + 1e-6 || nNeed > nMost)
                            rMutable.getGroupsThatDidNotFit().insert(aGroup);
                        if (fFont > oFactor->second + 1e-6)
                            continue;
                        // the node grows in steps of a thirty-second of the room the factor gives
                        const sal_Int64 nStep(
                            std::max<sal_Int64>(1, (nMost - nOwnHeight) / 32));
                        nNeed = nOwnHeight + ((nNeed - nOwnHeight + nStep - 1) / nStep) * nStep;
                        const sal_Int32 nHeight(static_cast<sal_Int32>(std::min<sal_Int64>(
                            std::min<sal_Int64>(nNeed, nMost), rShape->getSize().Height)));
                        aTextHeightAcross[pChild.get()] = nHeight;
                        aGrownByFactor.insert(pChild.get());
                        continue;
                    }
                    // A text stated with a length, "h for ch ptType node val 20", grows from it
                    // in steps of a thirty-second of it: Bar_List's bars are 118, 136, 37 and
                    // 136 thirty-seconds of their 20 mm.
                    if (!aStatedHeightOf.count(pChild->getInternalName()))
                    {
                        const auto aStated = aProperties.find(pChild->getInternalName());
                        aStatedHeightOf[pChild->getInternalName()]
                            = aStated != aProperties.end() && aStated->second.count(XML_h)
                                  ? aStated->second.at(XML_h)
                                  : 0;
                    }
                    if (const sal_Int64 nStated = aStatedHeightOf[pChild->getInternalName()])
                    {
                        const sal_Int64 nStep(std::max<sal_Int64>(1, nStated / 32));
                        nNeed = nNeed <= nStated
                                    ? nStated
                                    : nStated + ((nNeed - nStated + nStep - 1) / nStep) * nStep;
                    }
                    if (nNeed <= 0)
                        continue;
                    // A text higher than the row does not fit where the row is the Diagram's
                    // own, as high as the frame, Bar_List's, and its group is a point smaller on
                    // the next pass; a row inside a column grows with its texts.
                    if (nNeed > rShape->getSize().Height && rDgm.getLayout()
                        && rDgm.getLayout()->getNode()
                        && rDgm.getLayout()->getNode()->getName() == rShape->getInternalName())
                        rMutable.getGroupsThatDidNotFit().insert(aGroup);
                    const sal_Int32 nHeight(static_cast<sal_Int32>(
                        std::min<sal_Int64>(nNeed, rShape->getSize().Height)));
                    aProperties[pChild->getInternalName()][XML_h] = nHeight;
                    aTextHeightAcross[pChild.get()] = nHeight;
                    if (rMutable.getGroupsThatShrank().count(aGroup))
                        rMutable.getFittedFontSizes()[pChild->getInternalName()] = fTextFont;
                }

                // the nodes grown by a factor are as high as the highest of them where the row
                // holds its nodes' heights equal, "h for ch ptType node op equ"
                if (!aGrownByFactor.empty()
                    && std::any_of(rConstraints.begin(), rConstraints.end(),
                                   [](const Constraint& rEqual) {
                                       return rEqual.mnType == XML_h && rEqual.mnFor == XML_ch
                                              && rEqual.mnPointType == XML_node
                                              && rEqual.mnOperator == XML_equ
                                              && rEqual.mnRefType == XML_none;
                                   }))
                {
                    sal_Int32 nHighest(0);
                    std::set<OUString> aNames;
                    for (const Shape* pGrown : aGrownByFactor)
                    {
                        nHighest = std::max(nHighest, aTextHeightAcross[pGrown]);
                        aNames.insert(pGrown->getInternalName());
                    }
                    for (const ShapePtr& pChild : rShape->getChildren())
                        if (aNames.count(pChild->getInternalName()))
                            aTextHeightAcross[pChild.get()] = nHighest;
                }

                // a child bound to be at least as high as a text of the row ("op gte") is as
                // high as that text grew
                if (!aTextHeightAcross.empty())
                {
                    rtl::Reference<svx::diagram::Point> xOwn;
                    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                        if (rEntry.second == rShape)
                        {
                            xOwn = rEntry.first;
                            break;
                        }
                    std::vector<Constraint> aBounds;
                    gatherDecidedBounds(rDgm, getLayoutNode(), xOwn, aBounds);
                    for (const ShapePtr& pChild : rShape->getChildren())
                        for (const Constraint& rBound : aBounds)
                        {
                            if (rBound.mnOperator != XML_gte || rBound.mnType != XML_h
                                || rBound.mnRefType != XML_h || rBound.mnFor != XML_ch
                                || rBound.mnRefFor != XML_ch
                                || rBound.msForName != pChild->getInternalName()
                                || rBound.msRefForName.isEmpty()
                                || rBound.msRefForName == rBound.msForName)
                                continue;
                            const double fFactor(rBound.mfFactor > 0.0 ? rBound.mfFactor : 1.0);
                            for (const ShapePtr& pText : rShape->getChildren())
                            {
                                if (pText->getInternalName() != rBound.msRefForName)
                                    continue;
                                const auto aGrown = aTextHeightAcross.find(pText.get());
                                if (aGrown == aTextHeightAcross.end())
                                    continue;
                                const sal_Int32 nAtLeast(static_cast<sal_Int32>(
                                    std::min<double>(aGrown->second * fFactor,
                                                     rShape->getSize().Height)));
                                const std::optional<sal_Int32> oNow(findProperty(
                                    aProperties, pChild->getInternalName(), XML_h));
                                const auto aOwnGrown = aTextHeightAcross.find(pChild.get());
                                const sal_Int32 nNow(aOwnGrown != aTextHeightAcross.end()
                                                         ? aOwnGrown->second
                                                         : oNow.value_or(0));
                                if (nAtLeast > nNow)
                                {
                                    aProperties[pChild->getInternalName()][XML_h] = nAtLeast;
                                    aTextHeightAcross[pChild.get()] = nAtLeast;
                                }
                            }
                        }
                }
            }

            // A child whose extent across the row is tied to its extent along it, "w for des
            // header refType h refFor des header op equ fact 4" handed down to the column, is
            // held to that tie after the row above fitted its width: its height is its width
            // over the factor, where its wish would make it more. Process_List's headers are
            // 4 times as wide as high; the row fitted four columns into the width, 1837422
            // each, and the column had shared its height out to 1073151 a box where the drawing
            // has 459355, the width over four, and the column standing in the middle.
            // Only a child that asks for the whole along the row is held so; one whose extent
            // along is a stated length, Vertical_Equation's sibTrans of 0.58 of the node with
            // its w equal to its h, has the tie the other way round, the width following.
            std::set<OUString> aTiedNames;
            if (nIncX || nIncY)
            {
                const sal_Int32 nTiedType(nIncX ? XML_h : XML_w);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const OUString& rName(pChild->getInternalName());
                    const std::optional<double> oAlong(aExtents.get(*pChild));
                    if (oAlong && *oAlong < 0.999)
                        continue;
                    for (const Constraint& rTie : rConstraints)
                    {
                        if (rTie.mnType != nTiedType || rTie.mnRefType != nAxisType
                            || rTie.msForName != rName || rTie.msRefForName != rName
                            || rTie.mnOperator != XML_equ || rTie.mfFactor <= 0.0)
                            continue;
                        const std::optional<sal_Int32> oAcross(
                            findProperty(aProperties, rName, nTiedType));
                        const double fAcross(oAcross ? *oAcross
                                                     : (nIncX ? rShape->getSize().Height
                                                              : rShape->getSize().Width));
                        const double fAlong(fAcross / rTie.mfFactor);
                        const std::optional<double> oWish(aExtents.get(*pChild));
                        if (fAlong > 1.0 && fAlong < nParentExtent
                            && (!oWish || *oWish * nParentExtent > fAlong))
                        {
                            aExtents.set(rName, fAlong / nParentExtent, false);
                            aTiedNames.insert(rName);
                        }
                    }
                }
                // what is held equal to a tied child, "h for des child refType h refFor des
                // header op equ", by a factor or not, follows it: Process_List's boxes and its
                // arrows of 0.35 of the header; and so does what is stated as a part of it with
                // no operator, Chevron_List's nodes of 0.83 of the big chevron, and the
                // transitions of -0.13 of it and -0.14 of a node, which walk the place back
                // the sibTrans equal to the parTrans equal to the header: a chain, so the pass
                // runs until nothing more follows
                for (bool bFollowed = !aTiedNames.empty(); bFollowed;)
                {
                    bFollowed = false;
                    for (const ShapePtr& pChild : rShape->getChildren())
                    {
                        const OUString& rName(pChild->getInternalName());
                        if (aTiedNames.count(rName))
                            continue;
                        for (const Constraint& rEqual : rConstraints)
                        {
                            if (rEqual.mnType != nAxisType || rEqual.mnRefType != nAxisType
                                || rEqual.msForName != rName
                                || !aTiedNames.count(rEqual.msRefForName)
                                || (rEqual.mnOperator != XML_equ && rEqual.mnOperator != XML_none)
                                || rEqual.mfFactor == 0.0)
                                continue;
                            const std::optional<double> oOfTied(
                                aExtents.stated(rEqual.msRefForName));
                            if (!oOfTied)
                                continue;
                            aExtents.set(rName, *oOfTied * rEqual.mfFactor, false);
                            aTiedNames.insert(rName);
                            bFollowed = true;
                            break;
                        }
                    }
                }
            }

            // A child that wishes nothing and holds nothing, a column laid out for a level of
            // the tree the data does not have, takes no room: Lined_List's vert2 beside each
            // text of the second level is for a third level, its tree has two, and the text
            // has its 0.785 of the width in the drawing where we gave the empty column half of
            // the row as if it asked for the whole.
            // A spacer is another matter, its room is what it is for even where its wish is
            // beyond us, Numbered_List's sp of a tenth of the font size.
            if (nIncX || nIncY)
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    if (aExtents.get(*pChild)
                        || pChild->getServiceName() != "com.sun.star.drawing.GroupShape"
                        || !pChild->getChildren().empty())
                        continue;
                    const auto aChildNode = aRowNodes.find(pChild->getInternalName());
                    const auto aChildPoint = aRowPoints.find(pChild.get());
                    const AlgAtom* pChildAlg(
                        aChildNode != aRowNodes.end() && aChildNode->second
                            ? algorithmOf(rDgm, *aChildNode->second,
                                          aChildPoint != aRowPoints.end()
                                              ? aChildPoint->second
                                              : rtl::Reference<svx::diagram::Point>())
                            : nullptr);
                    if (pChildAlg && pChildAlg->getType() != XML_sp)
                        aExtents.set(pChild->getInternalName(), 0.0, false);
                }

            // A line across nothing that asks for the whole of the row lies behind the row,
            // along the whole of it, and takes no room in the flow: Lined_List's thickLine,
            // "w refType w" and "h" of nothing, runs the 8128000 of the row above the text of
            // 0.2 and the column beside it, which stand from the row's start as if the line
            // were not there. Continuous_Arrow_Process reaches the same by a walk back of the
            // whole; a row without one is this.
            std::set<OUString> aLinesBehind;
            if (nIncX || nIncY)
            {
                const sal_Int32 nAcrossType(nIncX ? XML_h : XML_w);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    if (pChild->getCustomShapeProperties()->getShapePresetType() != XML_line)
                        continue;
                    const std::optional<double> oWish(aExtents.get(*pChild));
                    if (!oWish || *oWish < 0.999)
                        continue;
                    const std::optional<sal_Int32> oAcross(
                        findProperty(aProperties, pChild->getInternalName(), nAcrossType));
                    if (oAcross && *oAcross > 0)
                        continue;
                    aExtents.set(pChild->getInternalName(), 0.0, false);
                    aLinesBehind.insert(pChild->getInternalName());
                }
            }

            // A child of a column that states no height of its own and is a row itself, a lin
            // that runs across, is as high as the tallest of its children, where a child states
            // its height as a part of its own width and the row states that width as a part of
            // its own: Sub-Step_Process's rows of text are 0.6 of their text box high, the box
            // 0.78 of the row, the row 0.77 of the column, 437656 each in the drawing, and not a
            // third of the column. The height is a length, the column stands at it.
            if (nIncY)
            {
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const std::optional<double> oStated(aExtents.get(*pChild));
                    if (oStated && *oStated < 0.999)
                        continue;
                    const auto aNode = aRowNodes.find(pChild->getInternalName());
                    if (aNode == aRowNodes.end() || !aNode->second)
                        continue;
                    const auto aPoint = aRowPoints.find(pChild.get());
                    const rtl::Reference<svx::diagram::Point> xRowPoint(
                        aPoint != aRowPoints.end() ? aPoint->second
                                                   : rtl::Reference<svx::diagram::Point>());
                    const AlgAtom* pRowAlg(algorithmOf(rDgm, *aNode->second, xRowPoint));
                    if (!pRowAlg || pRowAlg->getType() != XML_lin)
                        continue;
                    const auto aRowDir = pRowAlg->getMap().find(XML_linDir);
                    const sal_Int32 nRowDir(aRowDir == pRowAlg->getMap().end() ? XML_fromL
                                                                               : aRowDir->second);
                    if (nRowDir != XML_fromL && nRowDir != XML_fromR)
                        continue;
                    std::vector<Constraint> aRowOwn;
                    gatherDecidedConstraints(rDgm, *aNode->second, xRowPoint, aRowOwn);
                    const std::optional<double> oAcross(
                        aExtents.statedAcross(pChild->getInternalName()));
                    const double fRowWidth((oAcross ? *oAcross : 1.0) * rShape->getSize().Width);
                    double fTallest(0.0);
                    for (const Constraint& rWidth : aRowOwn)
                    {
                        if (rWidth.mnType != XML_w || rWidth.mnFor != XML_ch
                            || rWidth.msForName.isEmpty() || rWidth.mnRefType != XML_w
                            || !rWidth.msRefForName.isEmpty() || rWidth.mfFactor <= 0.0)
                            continue;
                        const auto aCell = aRowNodes.find(rWidth.msForName);
                        if (aCell == aRowNodes.end() || !aCell->second)
                            continue;
                        for (const Constraint& rOwn : collectDirectConstraints(*aCell->second))
                            if (rOwn.mnType == XML_h && rOwn.mnRefType == XML_w
                                && rOwn.msForName.isEmpty() && rOwn.msRefForName.isEmpty()
                                && rOwn.mfFactor > 0.0)
                                fTallest = std::max(fTallest,
                                                    fRowWidth * rWidth.mfFactor * rOwn.mfFactor);
                    }
                    if (fTallest > 0.0 && fTallest < nParentExtent)
                        aExtents.set(pChild->getInternalName(), fTallest / nParentExtent, false);
                }
            }

            // a spacer whose wish is a part of a shape a hierarchy in this row lays out gets
            // its extent once that is done, see settleDeferredSpacers
            for (const auto& rDeferred : aExtents.fitDeferred())
                for (size_t nChild = 0; nChild < rShape->getChildren().size(); ++nChild)
                {
                    const ShapePtr& pChild(rShape->getChildren()[nChild]);
                    if (pChild->getInternalName() == rDeferred.first
                        && pChild->getServiceName() == "com.sun.star.drawing.GroupShape"
                        && pChild->getChildren().empty())
                        const_cast<SmartArtDiagram&>(rDgm).getDeferredSpacers().push_back(
                            { rShape, pChild, static_cast<sal_Int32>(nChild),
                              rDeferred.second.msRefForName, rDeferred.second.mnRefType,
                              rDeferred.second.mfFactor, nIncX != 0,
                              (nIncX == -1 || nIncY == -1) ? -1 : 1 });
                }

            // A child that is a composite with an ar, width to height, is no larger along the
            // row than its extent across allows: Vertical_Accent_List's column gives its
            // chevron composite of ar 6 the width over 6 for a height, its text composite of
            // ar 11 0.9 of the width over 11, its parallelogram composite of ar 50 the width
            // over 50, whatever share of the height they asked for, and the fill scales all of
            // them by one factor. The cap is a length, so the row fills with it.
            std::set<OUString> aCappedByAspect;
            if (nIncX || nIncY)
            {
                const sal_Int32 nOtherExtent(nIncX ? rShape->getSize().Height
                                                   : rShape->getSize().Width);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const double fAspect(pChild->getAspectRatio());
                    if (fAspect <= 0.0 || nOtherExtent <= 0)
                        continue;
                    const std::optional<double> oAcross(
                        aExtents.statedAcross(pChild->getInternalName()));
                    const double fAcross((oAcross ? *oAcross : 1.0) * nOtherExtent);
                    const double fCap((nIncX ? fAcross * fAspect : fAcross / fAspect)
                                      / nParentExtent);
                    const std::optional<double> oAlong(aExtents.get(*pChild));
                    if (fCap > 0.0 && (!oAlong || *oAlong > fCap))
                    {
                        aExtents.set(pChild->getInternalName(), fCap, false);
                        aCappedByAspect.insert(pChild->getInternalName());
                    }
                }
            }

            // A child that states no extent of its own, or asks for the whole of the parent's,
            // may be a composite that states the extent of its own children in lengths, a band
            // 1.2 userA high or a spacer of userB less a part of userA. That is as far as the
            // child needs to reach, and the whole is only what it may have at most, so the length
            // is taken for the child's extent: a name of the layout's own, or an offset on one.
            if (nIncX || nIncY)
            {
                std::map<OUString, const LayoutNode*> aLayoutNodes;
                gatherLayoutNodes(mrLayoutNode, aLayoutNodes);
                std::map<const Shape*, rtl::Reference<svx::diagram::Point>> aPoints;
                for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                    if (rEntry.second)
                        aPoints[rEntry.second.get()] = rEntry.first;
                const sal_Int32 nOffsetType(nAxisType == XML_h ? XML_hOff : XML_wOff);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const std::optional<double> oStated(aExtents.get(*pChild));
                    const auto aNode = aLayoutNodes.find(pChild->getInternalName());
                    if (aNode == aLayoutNodes.end() || !aNode->second)
                        continue;
                    // a child that states an extent below the whole keeps it; one that states
                    // none is a spacer or the like and is read the same way as one that asks for
                    // the whole
                    if (oStated && *oStated < 0.999)
                        continue;
                    const auto aPoint = aPoints.find(pChild.get());
                    std::vector<Constraint> aOwn;
                    gatherDecidedConstraints(rDgm, *aNode->second,
                                             aPoint != aPoints.end()
                                                 ? aPoint->second
                                                 : rtl::Reference<svx::diagram::Point>(),
                                             aOwn);
                    double fReach(0.0);
                    bool bAny(false);
                    std::set<OUString> aSizedByName;
                    for (const Constraint& rOwn : aOwn)
                    {
                        if ((rOwn.mnType != nAxisType && rOwn.mnType != nOffsetType)
                            || rOwn.mnFor != XML_ch || !isUserVariable(rOwn.mnRefType))
                            continue;
                        const std::optional<sal_Int32> aValue(
                            resolveUserVariable(rDgm, rOwn.mnRefType, rConstraints));
                        if (!aValue)
                            continue;
                        fReach += *aValue * (rOwn.mfFactor != 0.0 ? rOwn.mfFactor : 1.0);
                        aSizedByName.insert(rOwn.msForName);
                        bAny = true;
                    }
                    // A child that states no extent of its own takes its content only where the
                    // content is nothing but such lengths, a spacer; a dot of a stated size beside
                    // a text says nothing about the width the text needs.
                    if (!oStated)
                        for (const ShapePtr& pInside : pChild->getChildren())
                            if (pInside->getSubType() != XML_conn
                                && !aSizedByName.count(pInside->getInternalName()))
                                bAny = false;
                    // A band that asked for the whole of the row and reaches 1.2 userA is still
                    // a share of the row: it is not a length the row may scale up to fill, for
                    // its content would not grow with it.
                    if (bAny && fReach > 0.0)
                        aExtents.set(pChild->getInternalName(), fReach / nParentExtent,
                                     oStated.has_value());
                }

                // A child that states no extent along the row and lays out a column across it
                // is as wide as the widest node of that column, and the column's nodes want a
                // height of their own that may far exceed the row: Vertical_Equation's root
                // says its nodes are 0.5 of its width high, three of them and two plus signs in
                // a column a third of that. The drawing scales the whole row by one factor, the
                // column's, so the nodes come out square as stated and the rest of the row
                // shrinks with them, and stands the row in the middle. The column's need is
                // read the same way the row reads its own wishes.
                // The columns are picked out before any of them gets a wish: the wish is set
                // by name, and Process_List's columns all carry one name.
                std::vector<ShapePtr> aColumnsToRead;
                for (const ShapePtr& pChild : rShape->getChildren())
                    if (!aExtents.get(*pChild))
                        aColumnsToRead.push_back(pChild);
                for (const ShapePtr& pChild : aColumnsToRead)
                {
                    const auto aNode = aLayoutNodes.find(pChild->getInternalName());
                    if (aNode == aLayoutNodes.end() || !aNode->second)
                        continue;
                    std::vector<const AlgAtom*> aAlgorithms;
                    gatherOwnAlgorithms(*aNode->second, aAlgorithms);
                    bool bColumnAcross(!aAlgorithms.empty());
                    for (const AlgAtom* pAlg : aAlgorithms)
                    {
                        const auto aDir = pAlg->getMap().find(XML_linDir);
                        const sal_Int32 nChildDir(aDir != pAlg->getMap().end() ? aDir->second
                                                                                : XML_fromL);
                        const bool bChildAlong(nChildDir == XML_fromL || nChildDir == XML_fromR);
                        if (pAlg->getType() != XML_lin || bChildAlong == bool(nIncX))
                            bColumnAcross = false;
                    }
                    if (!bColumnAcross)
                        continue;

                    std::map<OUString, const LayoutNode*> aBelow;
                    gatherLayoutNodes(*aNode->second, aBelow);
                    double fWidest(0.0);
                    for (const auto& rEntry : aBelow)
                    {
                        const std::optional<double> oStated(aExtents.stated(rEntry.first));
                        if (oStated)
                            fWidest = std::max(fWidest, *oStated);
                    }
                    // What the column's children need, read from the leaves up, counts like a
                    // size stated below: Table_Hierarchy's row of a node's children states the
                    // gap below each child, 0.084, and nothing for the child itself, whose node
                    // is as high as the node above by userH.
                    {
                        bool bGrown(false);
                        bool bFromBelow(false);
                        const std::optional<double> oNeed(
                            aNeed(pChild, aExtents, bGrown, bFromBelow));
                        if (bFromBelow && oNeed)
                            fWidest = std::max(fWidest, *oNeed);
                    }
                    if (fWidest <= 0.0)
                        continue;

                    const sal_Int32 nCrossExtent(nIncX ? rShape->getSize().Height
                                                       : rShape->getSize().Width);
                    std::vector<Constraint> aColumnConstraints(rConstraints);
                    const std::vector<Constraint> aOwn(collectDirectConstraints(*aNode->second));
                    aColumnConstraints.insert(aColumnConstraints.end(), aOwn.begin(), aOwn.end());
                    LinearChildExtents aColumn;
                    aColumn.read(rDgm, *aNode->second, pChild, aColumnConstraints,
                                 nIncX ? XML_h : XML_w, nCrossExtent, nParentExtent);
                    // The need is how far the walk along the column reaches, a wish of less
                    // than nothing walking it back: Chevron_List's rows of a big chevron, its
                    // nodes 0.83 of it and transitions of -0.13 and -0.14 reach 3.13 widths,
                    // and the drawing has them all at 0.32, where the positive wishes alone
                    // would make 3.49.
                    double fNeed(0.0);
                    double fAt(0.0);
                    for (const ShapePtr& pInside : pChild->getChildren())
                    {
                        const std::optional<double> oOne(aColumn.get(*pInside));
                        if (oOne)
                        {
                            fAt += *oOne;
                            fNeed = std::max(fNeed, fAt);
                        }
                    }
                    // A column whose nodes are shares of a height, Lined_List's rows of the
                    // whole, is scaled along itself like any row; only lengths across it, a
                    // node 0.5 of a width high, pull the row along.
                    if (fNeed > 1.0 && !aColumn.hasShareOfParent())
                        fUniformFit = std::min(fUniformFit, 1.0 / fNeed);
                    // Columns whose wishes are shares of one height, Process_List's header and
                    // its children each the whole, all scale by the fullest column: the header
                    // is the same in every column, and a column with fewer children stops
                    // short. The row gives such a column across only the part of the height
                    // its need is of the largest, so its own scaling comes out the same.
                    if (fNeed > 1.0 && aColumn.hasShareOfParent())
                    {
                        aShareNeedOfColumn[pChild.get()] = fNeed;
                        fLargestShareNeed = std::max(fLargestShareNeed, fNeed);
                    }
                    aExtents.set(pChild->getInternalName(), fWidest, false);
                }
            }
            // The wishes are handed out as parts of the parent, a spacer of a little less than
            // nothing among them as well: it pulls the next child a little closer, and the whole
            // is what the wishes add up to, Detailed_Process's four nodes and three gaps of 0.035
            // of a node. A layout that walks the place back over a whole child means its
            // children to overlap, and then the wishes are no plain division of the parent.
            const bool bDivideParent = aExtents.isFilled() && !aExtents.hasOverlap();
            std::vector<sal_Int32> aWantedExtent;

            // A row whose wishes walk back past the start of the child before means the next
            // child to stand over more than that one, and every child has a wish: Stacked_List's
            // posSpace of 0.4, vertFlow of 0.75, negSpace of -1.15 and circle of 0.5 put the
            // circle at the start of the node, over the column's left edge, and the transSpace
            // of 0.75 leads to the next node. The children are laid along in that order, a wish
            // of less than nothing moving the place for the next child back, and the unit is
            // the width over the farthest the walk reaches, the last node's column here. A wish
            // that walks back over a part of the child before, two neighbours lapping over each
            // other, is left to the paths below, and so is a row with a child as wide as the
            // whole row among the wishes, a layer behind the others: Continuous_Arrow_Process's
            // arrow of the whole width behind its nodes, reached by a walk back of the whole.
            // smallest shrink factor a column of this row found on a pass before, 1 where none did
            double fColumnFit(1.0);
            if (nIncX)
                for (const auto& pChild : rShape->getChildren())
                {
                    const auto aFit
                        = const_cast<SmartArtDiagram&>(rDgm).getColumnFits().find(
                            pChild->getInternalName());
                    if (aFit != const_cast<SmartArtDiagram&>(rDgm).getColumnFits().end())
                        fColumnFit = std::min(fColumnFit, aFit->second);
                }
            // a shrunk walk stands in the middle of the row
            sal_Int32 nWalkStart(0);
            bool bWalkBack(false);
            if (!bDivideParent && aExtents.hasBackwards())
            {
                bool bAllKnown(true);
                bool bPastTheOneBefore(false);
                bool bWholeAmongThem(false);
                bool bWalkBackOfTheWhole(false);
                double fBefore(0.0);
                double fAt(0.0);
                double fReach(0.0);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const std::optional<double> oWish(aExtents.get(*pChild));
                    if (!oWish)
                    {
                        bAllKnown = false;
                        break;
                    }
                    if (*oWish < 0.0 && -*oWish > fBefore + 1e-6)
                        bPastTheOneBefore = true;
                    if (*oWish >= 1.0 - 1e-6)
                        bWholeAmongThem = true;
                    if (*oWish <= -(1.0 - 1e-6))
                        bWalkBackOfTheWhole = true;
                    fAt += *oWish;
                    fReach = std::max(fReach, fAt);
                    fBefore = *oWish;
                }
                // A child as wide as the whole is a layer behind the others where a walk back of
                // the whole reaches it. Where the walk only laps the neighbours over each other
                // a little and reaches past the row all the same, the row is the walk, scaled
                // to the room: Chevron_List's big chevron is the whole width, its nodes 0.83
                // of it and its transitions -0.13 and -0.14, 3.13 widths in all, and the
                // drawing has them all at 0.32.
                bWalkBack = bAllKnown && (bPastTheOneBefore || fReach > 1.0 + 1e-6)
                            && !(bWholeAmongThem && bWalkBackOfTheWhole);
            }

            // first approximation of children size
            std::set<OUString> aChildrenToShrink;
            for (const auto& rRule : rRules)
            {
                // Consider rules: when scaling down, only change children where the rule allows
                // doing so.
                aChildrenToShrink.insert(rRule.msForName);
            }

            if (nDir == XML_fromT || nDir == XML_fromB)
            {
                // TODO consider rules for vertical linear layout as well.
                aChildrenToShrink.clear();
            }

            if (!aChildrenToShrink.empty())
            {
                // Have scaling info from rules: then only count scaled children.
                // Also count children which are a fraction of a scaled child.
                std::set<OUString> aChildrenToShrinkDeps;
                for (auto& aCurrShape : rShape->getChildren())
                {
                    if (aChildrenToShrink.find(aCurrShape->getInternalName())
                        == aChildrenToShrink.end())
                    {
                        if (fCount > 1.0)
                        {
                            fCount -= 1.0;

                            bool bIsDependency = false;
                            double fFactor = 0;
                            for (const auto& rConstraint : rConstraints)
                            {
                                if (rConstraint.msForName != aCurrShape->getInternalName())
                                {
                                    continue;
                                }

                                if ((nDir == XML_fromL || nDir == XML_fromR) && rConstraint.mnType != XML_w)
                                {
                                    continue;
                                }
                                if ((nDir == XML_fromL || nDir == XML_fromR) && rConstraint.mnType == XML_w)
                                {
                                    fFactor = rConstraint.mfFactor;
                                }

                                if ((nDir == XML_fromT || nDir == XML_fromB) && rConstraint.mnType != XML_h)
                                {
                                    continue;
                                }
                                if ((nDir == XML_fromT || nDir == XML_fromB) && rConstraint.mnType == XML_h)
                                {
                                    fFactor = rConstraint.mfFactor;
                                }

                                if (aChildrenToShrink.find(rConstraint.msRefForName) == aChildrenToShrink.end())
                                {
                                    continue;
                                }

                                // At this point we have a child with a size which is a factor of an
                                // other child which will be scaled.
                                fCount += rConstraint.mfFactor;
                                aChildrenToShrinkDeps.insert(aCurrShape->getInternalName());
                                bIsDependency = true;
                                break;
                            }

                            if (!bIsDependency && aCurrShape->getServiceName() == "com.sun.star.drawing.GroupShape")
                            {
                                bool bScaleDownEmptySpacing = false;
                                if (nDir == XML_fromL || nDir == XML_fromR)
                                {
                                    std::optional<sal_Int32> oWidth = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);
                                    bScaleDownEmptySpacing = oWidth.has_value() && oWidth.value() > 0;
                                }
                                if (!bScaleDownEmptySpacing && (nDir == XML_fromT || nDir == XML_fromB))
                                {
                                    std::optional<sal_Int32> oHeight = findProperty(aProperties, aCurrShape->getInternalName(), XML_h);
                                    bScaleDownEmptySpacing = oHeight.has_value() && oHeight.value() > 0;
                                }
                                if (bScaleDownEmptySpacing && aCurrShape->getChildren().empty())
                                {
                                    fCount += fFactor;
                                    aChildrenToShrinkDeps.insert(aCurrShape->getInternalName());
                                }
                            }
                        }
                    }
                }

                aChildrenToShrink.insert(aChildrenToShrinkDeps.begin(), aChildrenToShrinkDeps.end());

                // No manual spacing: spacings are children as well.
                aSpaceSize = awt::Size();

                // Where every child states its wish and a walk back of the whole with the child of
                // the whole after it lies behind the row, the children the rules let shrink take
                // what the others leave, one factor for all of them, and so does a child stated as
                // a part of one of them; every other child keeps its part of the row, and the two
                // behind stand as they are: Continuous_Arrow_Process's arrow is exact, its paddings
                // of 0.08 of the row and of userP stay, its four linV and the spaces of 0.2 of a
                // linV between them share the rest, 4023 each in the drawing where equal shares
                // made them 3975.
                bool bAllWish(true);
                for (const auto& rChild : rShape->getChildren())
                    if (!aExtents.get(*rChild))
                        bAllWish = false;
                if (bAllWish)
                {
                    const std::vector<ShapePtr>& rChildren(rShape->getChildren());
                    std::vector<bool> aBehind(rChildren.size(), false);
                    for (size_t n = 0; n + 1 < rChildren.size(); ++n)
                        if (*aExtents.get(*rChildren[n]) <= -(1.0 - 1e-6)
                            && *aExtents.get(*rChildren[n + 1]) >= 1.0 - 1e-6)
                            aBehind[n] = aBehind[n + 1] = true;
                    std::set<OUString> aNamedByRule;
                    for (const Rule& rRule : rRules)
                        if (rRule.mnType == nAxisType && !rRule.msForName.isEmpty())
                            aNamedByRule.insert(rRule.msForName);
                    const auto aShrinks = [&](const ShapePtr& rChild) {
                        if (aNamedByRule.count(rChild->getInternalName()))
                            return true;
                        for (const Constraint& rConstraint : rConstraints)
                            if (rConstraint.mnType == nAxisType
                                && rConstraint.msForName == rChild->getInternalName()
                                && aNamedByRule.count(rConstraint.msRefForName))
                                return true;
                        return false;
                    };
                    double fKeep(0.0), fShrink(0.0);
                    for (size_t n = 0; n < rChildren.size(); ++n)
                    {
                        if (aBehind[n])
                            continue;
                        (aShrinks(rChildren[n]) ? fShrink : fKeep) += *aExtents.get(*rChildren[n]);
                    }
                    if (std::find(aBehind.begin(), aBehind.end(), true) != aBehind.end()
                        && fShrink > 0.0 && fKeep < 1.0)
                    {
                        const double fFactor(std::min(1.0, (1.0 - fKeep) / fShrink));
                        for (size_t n = 0; n < rChildren.size(); ++n)
                        {
                            const double fWish(*aExtents.get(*rChildren[n]));
                            aWantedExtent.push_back(static_cast<sal_Int32>(
                                fWish * nParentExtent
                                * (!aBehind[n] && aShrinks(rChildren[n]) ? fFactor : 1.0)));
                        }
                    }
                }
            }
            else if (bWalkBack)
            {
                // A wish stated as a bare length keeps that length, the parts take the rest:
                // Basic_Chevron_Process's chevrons of the whole width lap over each other by a
                // space of -6 mm, and the drawing has them 6085 wide for a space of 600 where
                // scaling the space with them makes 5759 and 153.
                aSpaceSize = awt::Size();
                double fReach(0.0);
                double fWalk(0.0);
                double fFixedReach(0.0);
                double fFixedWalk(0.0);
                for (const auto& rChild : rShape->getChildren())
                {
                    const double fWish(*aExtents.get(*rChild));
                    if (aExtents.isFixedLength(rChild->getInternalName()))
                        fFixedWalk += fWish;
                    else
                        fWalk += fWish;
                    if (fWalk + fFixedWalk > fReach + fFixedReach)
                    {
                        fReach = fWalk;
                        fFixedReach = fFixedWalk;
                    }
                }
                const double fRoom(nParentExtent * (1.0 - fFixedReach));
                const double fUnit(fReach > 0.0 && fRoom > 0.0 ? fRoom / fReach * fColumnFit
                                                               : 0.0);
                nWalkStart = static_cast<sal_Int32>(fRoom * (1.0 - fColumnFit) / 2.0);
                for (const auto& rChild : rShape->getChildren())
                    aWantedExtent.push_back(static_cast<sal_Int32>(
                        *aExtents.get(*rChild)
                        * (aExtents.isFixedLength(rChild->getInternalName()) ? nParentExtent
                                                                              : fUnit)));
            }
            else if (!bDivideParent)
            {
                // TODO Handle spacing from constraints without rules as well.
                std::erase_if(
                    rShape->getChildren(),
                                   [](const ShapePtr& aChild) {
                                       return aChild->getServiceName()
                                                  == "com.sun.star.drawing.GroupShape"
                                              && aChild->getChildren().empty();
                                   });
                fCount = rShape->getChildren().size();
            }

            // The children whose size across the axis the constraints of this layout node have
            // already settled. A child only gets to state that size for itself where they have
            // not.
            const sal_Int32 nCrossType = nIncX ? XML_h : XML_w;
            // A node's extent across the row stated as a bare value is a length in millimetres:
            // Rainbow's bars are "h for ch ptType node val 20", 720000 EMU high, in a row given
            // the whole height, and stand at its bottom by vertAlign b.
            for (const Constraint& rConstraint : rConstraints)
            {
                if (rConstraint.mnType != nCrossType || rConstraint.mnFor != XML_ch
                    || rConstraint.mnRefType != XML_none || !std::isfinite(rConstraint.mfValue)
                    || rConstraint.mfValue <= 0.0 || !rConstraint.msRefForName.isEmpty())
                    continue;
                const sal_Int32 nLength(std::min<sal_Int32>(
                    nIncX ? rShape->getSize().Height : rShape->getSize().Width,
                    o3tl::convert(rConstraint.mfValue, o3tl::Length::mm, o3tl::Length::emu)));
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const bool bNamed(!rConstraint.msForName.isEmpty()
                                      && rConstraint.msForName == pChild->getInternalName());
                    const bool bByType(rConstraint.msForName.isEmpty()
                                       && (rConstraint.mnPointType == XML_all
                                           || (rConstraint.mnPointType == XML_node
                                               && pChild->getSubType() != XML_conn
                                               && pChild->getDataNodeType() != XML_sibTrans)));
                    if ((bNamed || bByType)
                        && !findProperty(aProperties, pChild->getInternalName(), nCrossType))
                        aProperties[pChild->getInternalName()][nCrossType] = nLength;
                }
            }
            std::set<OUString> aCrossSettledHere;
            for (const auto& rChild : rShape->getChildren())
            {
                if (findProperty(aProperties, rChild->getInternalName(), nCrossType).has_value())
                    aCrossSettledHere.insert(rChild->getInternalName());
            }

            std::map<OUString, std::vector<Constraint>> aChildConstraints;
            collectChildConstraints(getLayoutNode(), aChildConstraints);

            // A child states the size it wants in constraints of its own, which the shape around
            // it does not carry. Those fill in for a child that the constraints so far leave with
            // no size, so that the scale below starts from the size that was asked for
            {
                const std::vector<Constraint> aOwnConstraints(
                    collectDirectConstraints(getLayoutNode()));

                for (const auto& rChild : aChildConstraints)
                {
                    if (!findProperty(aProperties, rChild.first, XML_w).has_value())
                    {
                        const std::optional<sal_Int32> oWidth(readRequestedSize(
                            rChild.second, aOwnConstraints, XML_w, rShape->getSize()));
                        if (oWidth.has_value() && oWidth.value() > 0)
                            aProperties[rChild.first][XML_w] = oWidth.value();
                    }
                    if (!findProperty(aProperties, rChild.first, XML_h).has_value())
                    {
                        const std::optional<sal_Int32> oHeight(readRequestedSize(
                            rChild.second, aOwnConstraints, XML_h, rShape->getSize()));
                        if (oHeight.has_value() && oHeight.value() > 0)
                            aProperties[rChild.first][XML_h] = oHeight.value();
                    }
                    // A child's own size as a part of a name of the layout's own, "h refType
                    // userA fact 0.015" on Nested_Target's outerSibTrans with userA the root's
                    // width, is that part of what the name is known to hold, and along the axis
                    // it is the child's wish, a length: the spacers between the children of a
                    // box are 121920 there and took a whole share each before.
                    for (const Constraint& rOwn : rChild.second)
                    {
                        if ((rOwn.mnType != XML_w && rOwn.mnType != XML_h)
                            || !rOwn.msForName.isEmpty() || !isUserVariable(rOwn.mnRefType)
                            || rOwn.mfFactor <= 0.0)
                            continue;
                        const std::optional<sal_Int32> oOf(
                            resolveUserVariable(rDgm, rOwn.mnRefType, rConstraints));
                        if (!oOf || *oOf <= 0)
                            continue;
                        const sal_Int32 nLength(static_cast<sal_Int32>(*oOf * rOwn.mfFactor));
                        if (!findProperty(aProperties, rChild.first, rOwn.mnType).has_value())
                            aProperties[rChild.first][rOwn.mnType] = nLength;
                        if (rOwn.mnType != nAxisType || nParentExtent <= 0)
                            continue;
                        const auto aChildShape = std::find_if(
                            rShape->getChildren().begin(), rShape->getChildren().end(),
                            [&rChild](const ShapePtr& p) {
                                return p->getInternalName() == rChild.first;
                            });
                        if (aChildShape != rShape->getChildren().end()
                            && !aExtents.get(**aChildShape))
                            aExtents.set(rChild.first, static_cast<double>(nLength) / nParentExtent,
                                         false);
                    }
                }
            }

            if (bDivideParent)
            {
                // The wishes cover the children, the ones standing in for spacing included, so
                // the extents are handed out in one go and every child is scaled the same way.
                aChildrenToShrink.clear();
                aSpaceSize = awt::Size();
                for (const auto& rChild : rShape->getChildren())
                {
                    const std::optional<double> oFactor(aExtents.get(*rChild));
                    if (oFactor.has_value())
                    {
                        aWantedExtent.push_back(oFactor.value() * nParentExtent * fUniformFit);
                        continue;
                    }

                    // Nothing states what this child wants along the axis, so keep the extent the
                    // constraints gave it the plain way, and the whole parent extent if they gave
                    // it none. A row of such children ends up with equal shares.
                    const std::optional<sal_Int32> oPlain(
                        findProperty(aProperties, rChild->getInternalName(), nAxisType));
                    aWantedExtent.push_back(oPlain.has_value() && oPlain.value() > 0
                                                ? oPlain.value()
                                                : nParentExtent);
                }
            }

            awt::Size aChildSize = rShape->getSize();
            if (nDir == XML_fromL || nDir == XML_fromR)
                aChildSize.Width /= fCount;
            else if (nDir == XML_fromT || nDir == XML_fromB)
                aChildSize.Height /= fCount;

            awt::Point aCurrPos(0, 0);
            if (nIncX == -1)
                aCurrPos.X = rShape->getSize().Width - aChildSize.Width;
            if (nIncY == -1)
                aCurrPos.Y = rShape->getSize().Height - aChildSize.Height;

            // See if children requested more than 100% space in total: scale
            // down in that case.
            awt::Size aTotalSize;
            for (const auto & aCurrShape : rShape->getChildren())
            {
                std::optional<sal_Int32> oWidth = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);
                std::optional<sal_Int32> oHeight = findProperty(aProperties, aCurrShape->getInternalName(), XML_h);
                awt::Size aSize = aChildSize;
                if (oWidth.has_value())
                    aSize.Width = oWidth.value();
                if (oHeight.has_value())
                    aSize.Height = oHeight.value();
                aTotalSize.Width += aSize.Width;
                aTotalSize.Height += aSize.Height;
            }

            aTotalSize.Width += (fCount-1) * aSpaceSize.Width;
            aTotalSize.Height += (fCount-1) * aSpaceSize.Height;

            const bool bFillAxis = !aWantedExtent.empty();
            // a column in a row keeps its stated width for its own children, and the row goes
            // on by what the column draws: the part of its widest child, times the part of that
            // child its drawn shapes and texts are stated with where the child is a row itself
            std::vector<double> aAdvancePart(aWantedExtent.size(), 1.0);
            if (bFillAxis && nIncX && !bWalkBack)
            {
                std::map<OUString, const LayoutNode*> aBelowNodes;
                gatherLayoutNodes(getLayoutNode(), aBelowNodes);
                const auto aPointOf = [&rDgm](const ShapePtr& rOne) {
                    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                        if (rEntry.second == rOne)
                            return rEntry.first;
                    return rtl::Reference<svx::diagram::Point>();
                };
                // the part of its width each child of rOne is stated with, "w for ch forName X
                // refType w", for the branch rOne's point decides
                const auto aWidthParts = [&](const ShapePtr& rOne) {
                    std::map<OUString, double> aParts;
                    const auto aNode = aBelowNodes.find(rOne->getInternalName());
                    if (aNode == aBelowNodes.end() || !aNode->second)
                        return aParts;
                    std::vector<Constraint> aOwn;
                    gatherDecidedConstraints(rDgm, *aNode->second, aPointOf(rOne), aOwn);
                    for (const Constraint& rOwn : aOwn)
                        if (rOwn.mnType == XML_w && rOwn.mnFor == XML_ch
                            && !rOwn.msForName.isEmpty() && rOwn.mnRefType == XML_w
                            && rOwn.msRefForName.isEmpty() && rOwn.mfFactor > 0.0)
                            aParts[rOwn.msForName] = rOwn.mfFactor;
                    return aParts;
                };
                const auto aLinDirectionOf = [&](const ShapePtr& rOne) -> std::optional<sal_Int32> {
                    const auto aNode = aBelowNodes.find(rOne->getInternalName());
                    if (aNode == aBelowNodes.end() || !aNode->second)
                        return std::nullopt;
                    const AlgAtom* pAlgorithm(algorithmOf(rDgm, *aNode->second, aPointOf(rOne)));
                    if (!pAlgorithm || pAlgorithm->getType() != XML_lin)
                        return std::nullopt;
                    const auto aDirection = pAlgorithm->getMap().find(XML_linDir);
                    return aDirection != pAlgorithm->getMap().end() ? aDirection->second
                                                                    : XML_fromL;
                };
                // a shape that draws something or holds text, a text with its box hidden too;
                // a bare spacer is a group
                const auto aHasGeometry = [](const ShapePtr& rOne) {
                    return rOne->getServiceName() != "com.sun.star.drawing.GroupShape";
                };
                size_t nAt(0);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    const size_t nIndex(nAt++);
                    const std::optional<sal_Int32> oDirection(aLinDirectionOf(pChild));
                    if (!oDirection || (*oDirection != XML_fromT && *oDirection != XML_fromB)
                        || pChild->getChildren().empty())
                        continue;
                    const std::map<OUString, double> aParts(aWidthParts(pChild));
                    double fDrawn(0.0);
                    for (const ShapePtr& pInside : pChild->getChildren())
                    {
                        const auto aPart = aParts.find(pInside->getInternalName());
                        if (aPart == aParts.end())
                            continue;
                        double fOfIt(aHasGeometry(pInside) ? 1.0 : 0.0);
                        const std::optional<sal_Int32> oInsideDirection(aLinDirectionOf(pInside));
                        if (oInsideDirection
                            && (*oInsideDirection == XML_fromL || *oInsideDirection == XML_fromR))
                        {
                            const std::map<OUString, double> aRowParts(aWidthParts(pInside));
                            fOfIt = 0.0;
                            for (const ShapePtr& pCell : pInside->getChildren())
                            {
                                const auto aCell = aRowParts.find(pCell->getInternalName());
                                if (aCell != aRowParts.end() && aHasGeometry(pCell))
                                    fOfIt += aCell->second;
                            }
                            fOfIt = std::min(fOfIt, 1.0);
                        }
                        fDrawn = std::max(fDrawn, aPart->second * fOfIt);
                    }
                    if (fDrawn > 0.0 && fDrawn < 0.999)
                        aAdvancePart[nIndex] = fDrawn;
                }
            }
            // the walk fills the row by its unit, nothing is left to spread or to centre
            if (bWalkBack)
            {
                if (nIncX)
                    aTotalSize.Width = nParentExtent;
                else
                    aTotalSize.Height = nParentExtent;
            }
            // Where the wishes fall short of the parent the row does one of three things. A row
            // that states a gap along its axis, sp or sibSp, stands the children that gap apart
            // at their wished sizes and puts the whole where horzAlign, or vertAlign in a
            // column, says, in the middle where it says nothing: Labeled_Hierarchy's rows of
            // 0.25 stand 0.4 of a row apart, the rest halved above and below. A row whose wishes
            // are all lengths, parts of other shapes or of a name of the layout's own, or worked
            // out from the children's content, scales them up to fill: Detailed_Process's column
            // of 0.667, 0.125 and 0.058 becomes 0.784, 0.147 and 0.069, and Labeled_Hierarchy's
            // bands of 0.227 become 0.3. A row with a share of its own extent among the wishes,
            // a box list's text of 0.7 of the row, keeps them where they are, at the start of
            // the flow unless the alignment says otherwise.
            sal_Int32 nFillGap = 0;
            sal_Int32 nFillStart = bWalkBack ? nWalkStart : 0;
            if (bFillAxis && !bWalkBack)
            {
                sal_Int32 nWanted = 0;
                for (size_t n = 0; n < aWantedExtent.size(); ++n)
                    nWanted += static_cast<sal_Int32>(aWantedExtent[n] * aAdvancePart[n]);
                if (nWanted < nParentExtent && aWantedExtent.size() >= 1)
                {
                    // A row that carries its spaces as children of its own, empty groups between
                    // the nodes, takes no sp between them on top: Labeled_Hierarchy's bands
                    // stand with their spComp between and fill the height, the sp of the layout
                    // is for its other rows.
                    const bool bSpacerChildren(std::any_of(
                        rShape->getChildren().begin(), rShape->getChildren().end(),
                        [](const ShapePtr& pChild) {
                            return pChild->getServiceName() == "com.sun.star.drawing.GroupShape"
                                   && pChild->getChildren().empty();
                        }));
                    std::optional<sal_Int32> oGap;
                    for (const Constraint& rConstraint : rConstraints)
                    {
                        if (bSpacerChildren)
                            break;
                        if ((rConstraint.mnType != XML_sp && rConstraint.mnType != XML_sibSp)
                            || rConstraint.mnRefType != nAxisType || rConstraint.mfFactor <= 0.0)
                            continue;
                        double fOf(1.0);
                        if (!rConstraint.msRefForName.isEmpty())
                        {
                            const std::optional<double> oOf(
                                aExtents.stated(rConstraint.msRefForName));
                            if (!oOf)
                                continue;
                            fOf = *oOf;
                        }
                        oGap = static_cast<sal_Int32>(rConstraint.mfFactor * fOf * nParentExtent);
                        break;
                    }
                    const sal_Int32 nGaps(static_cast<sal_Int32>(aWantedExtent.size()) - 1);
                    // a gap the row has no room for is none
                    if (oGap && nWanted + nGaps * *oGap > nParentExtent)
                        oGap.reset();
                    // Only a column of nothing but spacers grows to fill: Detailed_Process's
                    // vSp1, simulatedConn and vSp2 of 0.8, 0.15 and 0.07 of the node beside them
                    // spread over its 1.2. A column with nodes in it stands at its wishes,
                    // Stacked_List's boxes of 0.667 of the column's width stay so however few
                    // there are.
                    const bool bAllSpacers(hasOnlySpacerChildren(getLayoutNode()));
                    if (oGap)
                        nFillGap = *oGap;
                    else if (bAllSpacers && !aExtents.hasShareOfParent() && nWanted > 0)
                    {
                        for (sal_Int32& rOne : aWantedExtent)
                            rOne = static_cast<sal_Int32>(
                                static_cast<double>(rOne) * nParentExtent / nWanted);
                        nWanted = 0;
                        for (sal_Int32 nOne : aWantedExtent)
                            nWanted += nOne;
                    }
                    const sal_Int32 nLeft(nParentExtent - nWanted - nGaps * nFillGap);
                    // A column whose wishes are all lengths, parts of other shapes and no share
                    // of the column, and whose own height the node above states, stands in its
                    // middle as well: Sub-Step_Process's rows of text, each as high as its text
                    // box, stand level with the step's circle in a column given the whole
                    // height. A column given no height, Stacked_List's, is as high as its
                    // content and the row above puts it where it says.
                    bool bLengthsInAStatedHeight(false);
                    if (nIncY && !aExtents.hasShareOfParent()
                        && !hasOnlySpacerChildren(getLayoutNode()))
                    {
                        const LayoutNode* pAbove(nullptr);
                        for (LayoutAtomPtr pUp = getLayoutNode().getParent(); pUp;
                             pUp = pUp->getParent())
                            if ((pAbove = dynamic_cast<const LayoutNode*>(pUp.get())))
                                break;
                        rtl::Reference<svx::diagram::Point> xOwnPoint;
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                            if (rEntry.second == rShape)
                            {
                                xOwnPoint = rEntry.first;
                                break;
                            }
                        if (pAbove && xOwnPoint.is())
                        {
                            const rtl::Reference<svx::diagram::Point> xAbovePoint(
                                rDgm.getData()->getPointByModelID(
                                    presentationParentOf(rDgm, xOwnPoint->msModelId)));
                            std::vector<Constraint> aAboveOwn;
                            gatherDecidedConstraints(rDgm, *pAbove, xAbovePoint, aAboveOwn);
                            for (const Constraint& rAbove : aAboveOwn)
                                if (rAbove.mnType == XML_h && rAbove.mnFor == XML_ch
                                    && rAbove.msForName == rShape->getInternalName()
                                    && rAbove.mnRefType == XML_h && rAbove.msRefForName.isEmpty()
                                    && rAbove.mfFactor > 0.0)
                                    bLengthsInAStatedHeight = true;
                        }
                    }
                    const sal_Int32 nUnsaid(oGap || fUniformFit < 1.0 || bLengthsInAStatedHeight
                                                ? (nIncX ? XML_ctr : XML_mid)
                                                : (nIncX ? XML_l : XML_t));
                    const sal_Int32 nAlongAlign(
                        nIncX ? (maMap.count(XML_horzAlign) ? maMap.find(XML_horzAlign)->second
                                                            : nUnsaid)
                              : (maMap.count(XML_vertAlign) ? maMap.find(XML_vertAlign)->second
                                                            : nUnsaid));
                    const bool bForward(nIncX == 1 || nIncY == 1);
                    const bool bAtStart(bForward ? (nAlongAlign == XML_l || nAlongAlign == XML_t)
                                                 : (nAlongAlign == XML_r || nAlongAlign == XML_b));
                    const bool bAtEnd(bForward ? (nAlongAlign == XML_r || nAlongAlign == XML_b)
                                               : (nAlongAlign == XML_l || nAlongAlign == XML_t));
                    if (nLeft > 0)
                        nFillStart = bAtStart ? 0 : bAtEnd ? nLeft : nLeft / 2;
                    nWanted = 0;
                    for (size_t n = 0; n < aWantedExtent.size(); ++n)
                        nWanted += static_cast<sal_Int32>(aWantedExtent[n] * aAdvancePart[n]);
                }
                if (nIncX)
                    aTotalSize.Width = nWanted;
                else
                    aTotalSize.Height = nWanted;
            }

            // A wish stated as a bare length keeps that length where the row shrinks the rest
            // to fit and a rule lets the others shrink and not it, "rule w for ch forName
            // composite val 0": Basic_Chevron_Process's chevrons of the whole width lap over
            // each other by a space of -6 mm, and the drawing has them 6085 wide for a space of
            // 600 where scaling the space with them makes 5759 and 153. Without such a rule the
            // length scales like everything else, Vertical_Chevron_List's sp of -14.88 mm
            // between composites of the whole height.
            const auto aKeepsItsLength = [&](const OUString& rName) {
                if (!bFillAxis || !aExtents.isFixedLength(rName))
                    return false;
                bool bOthersMayShrink(false);
                for (const Rule& rRule : rRules)
                {
                    if (rRule.mnType != nAxisType)
                        continue;
                    if (rRule.msForName == rName)
                        return false;
                    bOthersMayShrink = true;
                }
                return bOthersMayShrink;
            };
            sal_Int32 nFixedLengths(0);
            {
                size_t nAt(0);
                for (const auto& rChild : rShape->getChildren())
                {
                    if (nAt < aWantedExtent.size() && aKeepsItsLength(rChild->getInternalName()))
                        nFixedLengths += aWantedExtent[nAt];
                    ++nAt;
                }
            }
            const auto aScaleToFit = [nFixedLengths](sal_Int32 nTotal, sal_Int32 nRoom) {
                if (nTotal <= nRoom)
                    return 1.0;
                if (nTotal - nFixedLengths > 0 && nRoom - nFixedLengths > 0)
                    return static_cast<double>(nRoom - nFixedLengths) / (nTotal - nFixedLengths);
                return static_cast<double>(nRoom) / nTotal;
            };
            double fWidthScale = 1.0;
            double fHeightScale = 1.0;
            if (nIncX)
                fWidthScale = aScaleToFit(aTotalSize.Width, rShape->getSize().Width);
            if (nIncY)
                fHeightScale = aScaleToFit(aTotalSize.Height, rShape->getSize().Height);
            // a column whose children do not fit its height shrinks the whole row on the next pass,
            // so all keep their proportions. Only where every child is as high as a part of a named
            // width (the column or a shape beside it in the row) or as a sibling, since only then a
            // narrower row makes the column lower
            const auto aHighByAWidth = [&rConstraints, &rShape](const ShapePtr& pChild) {
                return std::any_of(
                    rConstraints.begin(), rConstraints.end(),
                    [&pChild, &rShape](const Constraint& rConstraint) {
                        if (rConstraint.mnType != XML_h
                            || rConstraint.msForName != pChild->getInternalName())
                            return false;
                        if (rConstraint.mnRefType == XML_w)
                            return !rConstraint.msRefForName.isEmpty();
                        return rConstraint.mnRefType == XML_h
                               && std::any_of(rShape->getChildren().begin(),
                                              rShape->getChildren().end(),
                                              [&rConstraint](const ShapePtr& pSibling) {
                                                  return pSibling->getInternalName()
                                                         == rConstraint.msRefForName;
                                              });
                    });
            };
            if (nIncY && fHeightScale < 1.0 - 1e-6
                && std::all_of(rShape->getChildren().begin(), rShape->getChildren().end(),
                               aHighByAWidth))
                if (const LayoutNode* pRowNode = getLayoutNode().getParentLayoutNode())
                {
                    std::vector<const AlgAtom*> aRowAlgorithms;
                    gatherOwnAlgorithms(*pRowNode, aRowAlgorithms);
                    if (!aRowAlgorithms.empty() && aRowAlgorithms.front()->getType() == XML_lin)
                    {
                        const auto aRowDirection
                            = aRowAlgorithms.front()->getMap().find(XML_linDir);
                        const sal_Int32 nRowDirection(
                            aRowDirection != aRowAlgorithms.front()->getMap().end()
                                ? aRowDirection->second
                                : XML_fromL);
                        if (nRowDirection == XML_fromL || nRowDirection == XML_fromR)
                        {
                            std::map<OUString, double>& rFits(
                                const_cast<SmartArtDiagram&>(rDgm).getColumnFitsOfThisPass());
                            const auto aFound = rFits.find(rShape->getInternalName());
                            rFits[rShape->getInternalName()]
                                = aFound != rFits.end() ? std::min(aFound->second, fHeightScale)
                                                        : fHeightScale;
                        }
                    }
                }
            // a walk has the part in its unit already
            if (nIncX && !bWalkBack)
                fWidthScale *= fColumnFit;
            aSpaceSize.Width *= fWidthScale;
            aSpaceSize.Height *= fHeightScale;

            sal_Int32 nAxisCursor = (nIncX == -1 || nIncY == -1) ? nParentExtent - nFillStart
                                                                 : nFillStart;
            size_t nWantedIndex = 0;
            for (auto& aCurrShape : rShape->getChildren())
            {
                // Extract properties relevant for this shape from constraints.
                std::optional<sal_Int32> oWidth = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);
                std::optional<sal_Int32> oHeight = findProperty(aProperties, aCurrShape->getInternalName(), XML_h);

                awt::Size aSize = aChildSize;
                if (oWidth.has_value())
                    aSize.Width = oWidth.value();
                if (oHeight.has_value())
                    aSize.Height = oHeight.value();
                // on a walk a wish of less than nothing is no size, it only moves the place
                // for the next child
                sal_Int32 nAdvance(0);
                if (bFillAxis)
                {
                    nAdvance = aWantedExtent[nWantedIndex];
                    const sal_Int32 nExtent(bWalkBack ? std::max<sal_Int32>(0, nAdvance)
                                                      : nAdvance);
                    if (nIncX)
                        aSize.Width = nExtent;
                    else
                        aSize.Height = nExtent;
                }
                ++nWantedIndex;
                const bool bKeepsItsLength(aKeepsItsLength(aCurrShape->getInternalName()));
                if ((aChildrenToShrink.empty()
                     || aChildrenToShrink.find(aCurrShape->getInternalName())
                            != aChildrenToShrink.end())
                    && !(nIncX && bKeepsItsLength))
                {
                    aSize.Width *= fWidthScale;
                }
                if ((aChildrenToShrink.empty()
                     || aChildrenToShrink.find(aCurrShape->getInternalName())
                            != aChildrenToShrink.end())
                    && !(nIncY && bKeepsItsLength))
                {
                    aSize.Height *= fHeightScale;
                }

                // A child whose wish along the row is a part of its own extent across, a
                // composite 0.4986 of its own height wide, keeps that ratio where the row shrinks
                // it along: Circle_Accent_Timeline's circles stay round and half the frame high.
                // Every other child is scaled along only, a text row's boxes keep their height.
                if (bFillAxis && aExtents.isTiedToOwnCross(aCurrShape->getInternalName()))
                {
                    if (nIncX && fWidthScale < 1.0)
                        aSize.Height *= fWidthScale;
                    if (nIncY && fHeightScale < 1.0)
                        aSize.Width *= fHeightScale;
                }

                {
                    const auto aShareNeed = aShareNeedOfColumn.find(aCurrShape.get());
                    if (aShareNeed != aShareNeedOfColumn.end() && fLargestShareNeed > 0.0
                        && aShareNeed->second < fLargestShareNeed)
                    {
                        const double fPart(aShareNeed->second / fLargestShareNeed);
                        if (nIncX)
                            aSize.Height = static_cast<sal_Int32>(aSize.Height * fPart);
                        else
                            aSize.Width = static_cast<sal_Int32>(aSize.Width * fPart);
                    }
                }

                // A layout may state every size of a row, along it and across it, as parts of a
                // name of its own, userH say, twice the height. The name is the unit of the whole
                // row then: where the row shrinks the children along it to fit, the unit shrank,
                // and a child's extent across that is a part of the same name shrinks the same
                // way. The unit as it came out is read off the child itself, its extent along
                // over the part of the name that extent was stated as; the extent across is its
                // own part of that, and not the room the child was clamped to.
                // The row's own extent across is such a unit as well: Vertical_Circle_List's
                // column states its composites as wide as the column and 0.909 of that high,
                // three of them and more far higher than the column, and the drawing has them
                // 0.238 of the width wide, in the middle, where they kept the whole width.
                {
                    std::optional<double> oAlongOfUnit;
                    std::optional<double> oAcrossOfUnit;
                    sal_Int32 nUnit(0);
                    for (const Constraint& rConstraint : rConstraints)
                    {
                        const bool bOwnAcross(rConstraint.mnRefType == nCrossType
                                              && rConstraint.mnRefFor == XML_self
                                              && rConstraint.msRefForName.isEmpty());
                        if (rConstraint.mnFor != XML_ch
                            || rConstraint.msForName != aCurrShape->getInternalName()
                            || (!isUserVariable(rConstraint.mnRefType) && !bOwnAcross)
                            || (nUnit && nUnit != rConstraint.mnRefType))
                            continue;
                        const double fFactor(rConstraint.mfFactor != 0.0 ? rConstraint.mfFactor
                                                                         : 1.0);
                        if (rConstraint.mnType == nAxisType && fFactor > 0.0)
                        {
                            oAlongOfUnit = fFactor;
                            nUnit = rConstraint.mnRefType;
                        }
                        else if (rConstraint.mnType == nCrossType && fFactor > 0.0)
                        {
                            oAcrossOfUnit = fFactor;
                            nUnit = rConstraint.mnRefType;
                        }
                    }
                    if (oAlongOfUnit && oAcrossOfUnit)
                    {
                        const double fUnit((nIncX ? aSize.Width : aSize.Height) / *oAlongOfUnit);
                        const sal_Int32 nAcross(static_cast<sal_Int32>(fUnit * *oAcrossOfUnit));
                        if (nIncX)
                            aSize.Height = std::min(nAcross, rShape->getSize().Height);
                        else
                            aSize.Width = std::min(nAcross, rShape->getSize().Width);
                    }
                }
                // A node that states its own size across the axis as a part of its size along
                // the axis can only be worked out here, where the size along the axis is
                // settled. It never grows past the room the parent has across the axis.
                // A node above may state a child's size across the axis as a part of its size
                // along it by name, "w for des sibTrans refType h refFor des sibTrans":
                // Vertical_Equation's plus signs are as wide as they are high, a square of 0.58
                // of a node. What the constraints settled for the width before was read against
                // the height as it stood then, so it is worked out here again.
                std::optional<double> oNamedCross;
                for (const Constraint& rConstraint : rConstraints)
                {
                    if (rConstraint.mnType != nCrossType || rConstraint.mnRefType != nAxisType
                        || rConstraint.msForName != aCurrShape->getInternalName()
                        || rConstraint.msRefForName != aCurrShape->getInternalName())
                        continue;
                    oNamedCross = rConstraint.mfFactor != 0.0 ? rConstraint.mfFactor : 1.0;
                }
                if (oNamedCross || !aCrossSettledHere.count(aCurrShape->getInternalName()))
                {
                    const auto aChild = aChildConstraints.find(aCurrShape->getInternalName());
                    if (oNamedCross || aChild != aChildConstraints.end())
                    {
                        std::optional<double> oCross(oNamedCross);
                        if (!oCross)
                            oCross = readOwnCrossFactor(aChild->second, nCrossType, nAxisType);
                        if (oCross.has_value())
                        {
                            if (nIncX)
                                aSize.Height = std::min<sal_Int32>(aSize.Width * oCross.value(),
                                                                   rShape->getSize().Height);
                            else
                                aSize.Width = std::min<sal_Int32>(aSize.Height * oCross.value(),
                                                                  rShape->getSize().Width);
                        }
                    }
                }

                // A node may bound its own size, one side against the other: Detailed_Process's
                // composite is at most 1.2 of its width high, and stands so in the drawing while
                // the row is far higher. The bound is read from the node's own constraints, the
                // branch of a choose decided by its point, and applied to what the row gave it.
                {
                    std::map<OUString, const LayoutNode*> aLayoutNodes;
                    gatherLayoutNodes(getLayoutNode(), aLayoutNodes);
                    const auto aChildNode = aLayoutNodes.find(aCurrShape->getInternalName());
                    const LayoutNode* pChildNode(
                        aChildNode != aLayoutNodes.end() ? aChildNode->second : nullptr);
                    if (pChildNode)
                    {
                        rtl::Reference<svx::diagram::Point> xChildPoint;
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                            if (rEntry.second == aCurrShape)
                            {
                                xChildPoint = rEntry.first;
                                break;
                            }
                        std::vector<Constraint> aOwnBounds;
                        gatherDecidedBounds(rDgm, *pChildNode, xChildPoint, aOwnBounds);
                        for (const Constraint& rBound : aOwnBounds)
                        {
                            if (!rBound.msForName.isEmpty() || !rBound.msRefForName.isEmpty()
                                || (rBound.mnType != XML_w && rBound.mnType != XML_h)
                                || (rBound.mnRefType != XML_w && rBound.mnRefType != XML_h)
                                || rBound.mfFactor <= 0.0)
                                continue;
                            const sal_Int32 nLimit(static_cast<sal_Int32>(
                                (rBound.mnRefType == XML_w ? aSize.Width : aSize.Height)
                                * rBound.mfFactor));
                            sal_Int32& rSide(rBound.mnType == XML_w ? aSize.Width : aSize.Height);
                            const sal_Int32 nBefore(rSide);
                            rSide = rBound.mnOperator == XML_lte ? std::min(rSide, nLimit)
                                                                 : std::max(rSide, nLimit);
                            if (rSide == nBefore)
                                continue;
                            // A child after this one may be stated as a part of the side that
                            // moved, the column beside the composite as high as it: it follows
                            // the bound, and reads the side as it stands now.
                            aProperties[aCurrShape->getInternalName()][rBound.mnType] = rSide;
                            for (const Constraint& rConstraint : rConstraints)
                            {
                                if (rConstraint.mnFor != XML_ch
                                    || rConstraint.msRefForName != aCurrShape->getInternalName()
                                    || rConstraint.mnRefType != rBound.mnType
                                    || (rConstraint.mnType != XML_w && rConstraint.mnType != XML_h)
                                    || rConstraint.msForName == aCurrShape->getInternalName())
                                    continue;
                                const double fFactor(rConstraint.mfFactor != 0.0
                                                         ? rConstraint.mfFactor
                                                         : 1.0);
                                aProperties[rConstraint.msForName][rConstraint.mnType]
                                    = static_cast<sal_Int32>(rSide * fFactor);
                            }
                        }
                    }
                }

                // A child capped by its ar keeps that shape: where the fill scaled it along the
                // row, its extent across follows, Vertical_Accent_List's chevron composite
                // 6104k wide for its 1017k of height.
                if (aCappedByAspect.count(aCurrShape->getInternalName())
                    && aCurrShape->getAspectRatio() > 0.0)
                {
                    if (nIncX)
                        aSize.Height = std::min<sal_Int32>(
                            rShape->getSize().Height,
                            static_cast<sal_Int32>(aSize.Width / aCurrShape->getAspectRatio()));
                    else
                        aSize.Width = std::min<sal_Int32>(
                            rShape->getSize().Width,
                            static_cast<sal_Int32>(aSize.Height * aCurrShape->getAspectRatio()));
                }

                // A line in a row lies along it: where nothing states its extent across the
                // row it has none, and the alignment below puts it in the middle.
                // Sub-Step_Process's preLine and postLine are 102867 long and flat in the
                // middle of their row of text; a row's height would draw them on the slant.
                if (aCurrShape->getCustomShapeProperties()->getShapePresetType() == XML_line
                    && !aCrossSettledHere.count(aCurrShape->getInternalName())
                    && !findProperty(aProperties, aCurrShape->getInternalName(), nCrossType)
                             .has_value())
                {
                    if (nIncX)
                        aSize.Height = 0;
                    else
                        aSize.Width = 0;
                }

                // a text that grows across a column with its words, as wide as they are, and
                // one that grows across a row with its lines, as high as they are
                if (nIncY)
                {
                    const auto aAcross = aTextWidthAcross.find(aCurrShape.get());
                    if (aAcross != aTextWidthAcross.end())
                        aSize.Width = aAcross->second;
                }
                if (nIncX)
                {
                    const auto aAcross = aTextHeightAcross.find(aCurrShape.get());
                    if (aAcross != aTextHeightAcross.end())
                        aSize.Height = aAcross->second;
                }

                aCurrShape->setSize(aSize);
                aCurrShape->setChildSize(aSize);
                // a column standing across this row reads the widths stated for its children
                // "for des" against the row
                const auto aColumnNode = aRowNodes.find(aCurrShape->getInternalName());
                if (nIncX && aSize.Width < rShape->getSize().Width
                    && aColumnNode != aRowNodes.end() && aColumnNode->second)
                {
                    std::vector<const AlgAtom*> aColumnAlgorithms;
                    gatherOwnAlgorithms(*aColumnNode->second, aColumnAlgorithms);
                    for (const AlgAtom* pColumnAlgorithm : aColumnAlgorithms)
                    {
                        const auto aDirection = pColumnAlgorithm->getMap().find(XML_linDir);
                        if (pColumnAlgorithm->getType() == XML_lin
                            && aDirection != pColumnAlgorithm->getMap().end()
                            && (aDirection->second == XML_fromT || aDirection->second == XML_fromB))
                            const_cast<SmartArtDiagram&>(rDgm)
                                .getWidthOfDescendantSizes()[aCurrShape.get()]
                                = rShape->getSize().Width;
                    }
                }

                // Across the axis a child stands where nodeVertAlign, or nodeHorzAlign in a
                // column, puts it: at the top, in the middle, or at the bottom of the row, and
                // in the middle where the layout says nothing. Random_to_Result_Process stands
                // its chevrons at the top of a row of taller composites.
                if (nIncX)
                {
                    const sal_Int32 nAlign(maMap.count(XML_nodeVertAlign)
                                               ? maMap.find(XML_nodeVertAlign)->second
                                               : XML_mid);
                    const sal_Int32 nRoom(rShape->getSize().Height - aSize.Height);
                    aCurrPos.Y = nAlign == XML_t ? 0 : nAlign == XML_b ? nRoom : nRoom / 2;
                }
                if (nIncY)
                {
                    const sal_Int32 nAlign(maMap.count(XML_nodeHorzAlign)
                                               ? maMap.find(XML_nodeHorzAlign)->second
                                               : XML_ctr);
                    const sal_Int32 nRoom(rShape->getSize().Width - aSize.Width);
                    aCurrPos.X = nAlign == XML_l ? 0 : nAlign == XML_r ? nRoom : nRoom / 2;
                    // a composite stands at the side with what it draws
                    if ((nAlign == XML_l || nAlign == XML_r) && !aCurrShape->getChildren().empty())
                    {
                        std::map<OUString, const LayoutNode*> aAlignNodes;
                        gatherLayoutNodes(getLayoutNode(), aAlignNodes);
                        const auto aAlignNode = aAlignNodes.find(aCurrShape->getInternalName());
                        rtl::Reference<svx::diagram::Point> xAlignPoint;
                        for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                            if (rEntry.second == aCurrShape)
                            {
                                xAlignPoint = rEntry.first;
                                break;
                            }
                        if (aAlignNode != aAlignNodes.end() && aAlignNode->second)
                        {
                            const DrawnParts aDrawn(drawnPartsOf(rDgm, *aAlignNode->second,
                                                                 xAlignPoint, aCurrShape));
                            if (nAlign == XML_l)
                                aCurrPos.X -= static_cast<sal_Int32>(aDrawn.fLeft * aSize.Width);
                            else
                                aCurrPos.X += static_cast<sal_Int32>((1.0 - aDrawn.fRight)
                                                                     * aSize.Width);
                        }
                    }
                }
                if (aCurrPos.X < 0 && !nIncY)
                {
                    aCurrPos.X = 0;
                }
                if (aCurrPos.Y < 0)
                {
                    aCurrPos.Y = 0;
                }

                if (bFillAxis)
                {
                    const double fAdvancePart(nWantedIndex >= 1
                                                  && nWantedIndex - 1 < aAdvancePart.size()
                                                  ? aAdvancePart[nWantedIndex - 1]
                                                  : 1.0);
                    const sal_Int32 nExtent
                        = bWalkBack ? nAdvance
                                    : static_cast<sal_Int32>((nIncX ? aSize.Width : aSize.Height)
                                                             * fAdvancePart);
                    if (nIncX == -1 || nIncY == -1)
                        nAxisCursor -= nExtent;
                    if (nIncX)
                        aCurrPos.X = nAxisCursor;
                    else
                        aCurrPos.Y = nAxisCursor;
                    // a column that goes on by what it draws has that in its middle, so its box
                    // stands back by half of the rest and what it draws fills its place
                    if (nIncX && !bWalkBack && fAdvancePart < 1.0)
                        aCurrPos.X -= (aSize.Width - nExtent) / 2;
                    if (nIncX == 1 || nIncY == 1)
                        nAxisCursor += nExtent + nFillGap;
                    else
                        nAxisCursor -= nFillGap;
                }

                aCurrShape->setPosition(aCurrPos);

                aCurrPos.X += nIncX * (aSize.Width + aSpaceSize.Width);
                aCurrPos.Y += nIncY * (aSize.Height + aSpaceSize.Height);

                // connectors should be handled in conn, but we don't have
                // reference to previous and next child, so it's easier here
                if (aCurrShape->getSubType() == XML_conn)
                    aCurrShape->setRotation(nConnectorAngle * PER_DEGREE);
            }

            // the lines behind the row span it from its start
            for (const ShapePtr& pChild : rShape->getChildren())
            {
                if (!aLinesBehind.count(pChild->getInternalName()))
                    continue;
                awt::Point aAt(pChild->getPosition());
                awt::Size aLine(pChild->getSize());
                if (nIncX)
                {
                    aAt.X = 0;
                    aLine.Width = rShape->getSize().Width;
                    aLine.Height = 0;
                }
                else
                {
                    aAt.Y = 0;
                    aLine.Height = rShape->getSize().Height;
                    aLine.Width = 0;
                }
                pChild->setPosition(aAt);
                pChild->setSize(aLine);
                pChild->setChildSize(aLine);
            }

            // Across the axis the row's content is a band as wide as its widest child, and the
            // row's own alignment across, vertAlign of a row and horzAlign of a column, puts the
            // band at the start, in the middle or at the end of the row; the nodes stand in the
            // band where nodeVertAlign or nodeHorzAlign says. Increasing_Circle_Process's row
            // says vertAlign t and its composites, 2447395 high in 5418667, stand at the top,
            // where the middle they were given by nodeVertAlign's default put them 1485k down.
            {
                const sal_Int32 nBandAlign(nIncX ? (maMap.count(XML_vertAlign)
                                                        ? maMap.find(XML_vertAlign)->second
                                                        : XML_mid)
                                                 : (maMap.count(XML_horzAlign)
                                                        ? maMap.find(XML_horzAlign)->second
                                                        : XML_ctr));
                const bool bBandAtStart(nBandAlign == XML_t || nBandAlign == XML_l);
                const bool bBandAtEnd(nBandAlign == XML_b || nBandAlign == XML_r);
                if (bBandAtStart || bBandAtEnd)
                {
                    // the band is the nodes': a transition between them may be as high as the
                    // row, Increasing_Circle_Process's arrows are
                    sal_Int32 nBand(0);
                    for (const ShapePtr& pChild : rShape->getChildren())
                    {
                        const auto aChildNode = aRowNodes.find(pChild->getInternalName());
                        const auto aChildPoint = aRowPoints.find(pChild.get());
                        const AlgAtom* pChildAlg(
                            aChildNode != aRowNodes.end() && aChildNode->second
                                ? algorithmOf(rDgm, *aChildNode->second,
                                              aChildPoint != aRowPoints.end()
                                                  ? aChildPoint->second
                                                  : rtl::Reference<svx::diagram::Point>())
                                : nullptr);
                        // nor a spacer's: Increasing_Circle_Process's sibTrans is a space of
                        // the row's whole height between its composites
                        if (pChildAlg
                            && (pChildAlg->getType() == XML_conn || pChildAlg->getType() == XML_sp))
                            continue;
                        if (pChild->getServiceName() == "com.sun.star.drawing.GroupShape"
                            && pChild->getChildren().empty())
                            continue;
                        nBand = std::max(nBand, nIncX ? pChild->getSize().Height
                                                      : pChild->getSize().Width);
                    }
                    if (nBand <= 0)
                        for (const ShapePtr& pChild : rShape->getChildren())
                            nBand = std::max(nBand, nIncX ? pChild->getSize().Height
                                                          : pChild->getSize().Width);
                    const sal_Int32 nBox(nIncX ? rShape->getSize().Height
                                               : rShape->getSize().Width);
                    const sal_Int32 nOffset(bBandAtStart ? 0
                                                         : std::max<sal_Int32>(0, nBox - nBand));
                    const sal_Int32 nNodeAlign(
                        nIncX ? (maMap.count(XML_nodeVertAlign)
                                     ? maMap.find(XML_nodeVertAlign)->second
                                     : XML_mid)
                              : (maMap.count(XML_nodeHorzAlign)
                                     ? maMap.find(XML_nodeHorzAlign)->second
                                     : XML_ctr));
                    for (const ShapePtr& pChild : rShape->getChildren())
                    {
                        const sal_Int32 nOwn(nIncX ? pChild->getSize().Height
                                                   : pChild->getSize().Width);
                        const sal_Int32 nWithin(
                            nNodeAlign == XML_t || nNodeAlign == XML_l
                                ? 0
                                : nNodeAlign == XML_b || nNodeAlign == XML_r
                                      ? nBand - nOwn
                                      : (nBand - nOwn) / 2);
                        awt::Point aAt(pChild->getPosition());
                        if (nIncX)
                            aAt.Y = nOffset + nWithin;
                        else
                            aAt.X = nOffset + nWithin;
                        pChild->setPosition(aAt);
                    }
                }
            }

            // Newer shapes are behind older ones by default. Reverse this if requested.
            sal_Int32 nChildOrder = XML_b;
            const LayoutNode* pParentLayoutNode = nullptr;
            for (LayoutAtomPtr pAtom = getParent(); pAtom; pAtom = pAtom->getParent())
            {
                auto pLayoutNode = dynamic_cast<LayoutNode*>(pAtom.get());
                if (pLayoutNode)
                {
                    pParentLayoutNode = pLayoutNode;
                    break;
                }
            }
            if (pParentLayoutNode)
            {
                nChildOrder = pParentLayoutNode->getChildOrder();
            }
            if (nChildOrder == XML_t)
            {
                std::reverse(rShape->getChildren().begin(), rShape->getChildren().end());
            }

            break;
        }

        case XML_pyra:
        {
            PyraAlg::layoutShapeChildren(rDgm, *this, rShape, rConstraints);
            break;
        }

        case XML_snake:
        {
            if (layoutTextDrivenSnake(rDgm, *this, rShape, rConstraints))
                break;
            SnakeAlg::layoutShapeChildren(rDgm, *this, rShape, rConstraints);
            break;
        }

        case XML_sp:
        {
            // HACK: Handled one level higher. Or rather, planned to
            // HACK: text should appear only in tx node; we're assigning it earlier, so let's remove it here
            rShape->setTextBody(TextBodyPtr());

            // A space can hold layout nodes of its own, a picture that repeats for each node
            // say, and those fill it: the space says nothing else about them.
            for (const ShapePtr& pChild : rShape->getChildren())
            {
                pChild->setPosition(awt::Point(0, 0));
                pChild->setSize(rShape->getSize());
                pChild->setChildSize(rShape->getSize());
            }
            break;
        }

        case XML_tx:
        {
            // adjust text alignment

            // Parse constraints, only self margins as a start.
            double fFontSize = 0;
            for (const auto& rConstr : rConstraints)
            {
                if (rConstr.mnRefType == XML_w)
                {
                    if (!rConstr.msForName.isEmpty())
                        continue;

                    sal_Int32 nProperty = getPropertyFromConstraint(rConstr.mnType);
                    if (!nProperty)
                        continue;

                    // PowerPoint takes size as points, but gives margin as MMs.
                    double fFactor = convertPointToMms(rConstr.mfFactor);

                    // DrawingML works in EMUs, UNO API works in MM100s.
                    sal_Int32 nValue = o3tl::convert(rShape->getSize().Width * fFactor,
                                                     o3tl::Length::emu, o3tl::Length::mm100);

                    rShape->getShapeProperties().setProperty(nProperty, nValue);
                }
                if (rConstr.mnType == XML_primFontSz)
                    fFontSize = rConstr.mfValue;
            }
            // the font a row fitted for a node whose box grows with its text
            bool bFitted(false);
            {
                const auto aFitted = const_cast<SmartArtDiagram&>(rDgm).getFittedFontSizes().find(
                    rShape->getInternalName());
                if (aFitted != const_cast<SmartArtDiagram&>(rDgm).getFittedFontSizes().end())
                {
                    fFontSize = aFitted->second;
                    bFitted = true;
                }
            }

            // a text in a box of a given size, no rule growing it, takes the font of its group on
            // this pass; when it does not fit, a word too wide or its lines too high, the group
            // is a point smaller on the next pass
            {
                const sal_Int32 nTurn(
                    (((rShape->getRotation() + rShape->getDiagramRotation()) / PER_DEGREE) % 360
                     + 360)
                    % 360);
                const bool bUpright(!maMap.count(XML_autoTxRot)
                                    || maMap.find(XML_autoTxRot)->second == XML_upr);
                // upr turns the text by a quarter, it then stands across its box
                const bool bAcross(bUpright
                                   && ((nTurn > 45 && nTurn < 135)
                                       || (nTurn > 225 && nTurn < 315)));
                if (!bFitted && !ownRuleGrows(getLayoutNode(), XML_h)
                    && !ownRuleFrees(getLayoutNode(), XML_w) && rShape->getSize().Width > 0
                    && rShape->getSize().Height > 0 && !textParagraphsOf(rDgm, rShape).empty())
                {
                    rtl::Reference<svx::diagram::Point> xPoint;
                    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
                        if (rEntry.second == rShape)
                        {
                            xPoint = rEntry.first;
                            break;
                        }
                    // the room the preset leaves the text, turned with the text; a shape its
                    // layout node turns by a quarter has the turned box here, so its own box lies
                    // the other way
                    const sal_Int32 nNodeTurn(
                        ((rShape->getDiagramRotation() / PER_DEGREE) % 360 + 360) % 360);
                    const bool bNodeQuarter((nNodeTurn > 45 && nNodeTurn < 135)
                                            || (nNodeTurn > 225 && nNodeTurn < 315));
                    double fRoomWidth(bNodeQuarter ? rShape->getSize().Height
                                                   : rShape->getSize().Width);
                    double fRoomHeight(bNodeQuarter ? rShape->getSize().Width
                                                    : rShape->getSize().Height);
                    if (const std::optional<std::array<double, 4>> oArea
                        = textAreaPartsOf(rShape, fRoomWidth, fRoomHeight))
                    {
                        fRoomWidth *= 1.0 - (*oArea)[0] - (*oArea)[2];
                        fRoomHeight *= 1.0 - (*oArea)[1] - (*oArea)[3];
                    }
                    if (bAcross)
                        std::swap(fRoomWidth, fRoomHeight);
                    bool bWordsFit(true);
                    const std::optional<sal_Int64> oNeed(textDrivenHeight(
                        rDgm, rConstraints, getLayoutNode(), xPoint, rShape,
                        static_cast<sal_Int64>(fRoomWidth), bWordsFit, /*bGrowingOnly*/ false));
                    OUString aGroup;
                    const double fGroupFont(
                        fontOfGroup(rDgm, rConstraints, rShape->getInternalName(), aGroup));
                    if (oNeed && *oNeed > fRoomHeight)
                        const_cast<SmartArtDiagram&>(rDgm).getGroupsThatDidNotFit().insert(aGroup);
                    if (oNeed)
                    {
                        fFontSize = fGroupFont;
                        // the shape keeps its text within the margins we fitted with, a side
                        // stated as a part of the width is set above
                        for (const sal_Int32 nSide : { XML_lMarg, XML_tMarg, XML_rMarg, XML_bMarg })
                        {
                            const TextMargin aMargin(
                                textMarginOf(rDgm, rConstraints, getLayoutNode(), xPoint,
                                             rShape->getInternalName(), nSide));
                            if (aMargin.mfPartOfWidth > 0.0
                                || std::any_of(rConstraints.begin(), rConstraints.end(),
                                               [nSide](const Constraint& rConstraint) {
                                                   return rConstraint.msForName.isEmpty()
                                                          && rConstraint.mnType == nSide
                                                          && rConstraint.mnRefType == XML_w;
                                               }))
                                continue;
                            const double fEmu(aMargin.mfPartOfFont * fGroupFont * 12700.0
                                              + aMargin.mnLength);
                            rShape->getShapeProperties().setProperty(
                                getPropertyFromConstraint(nSide),
                                static_cast<sal_Int32>(o3tl::convert(
                                    fEmu, o3tl::Length::emu, o3tl::Length::mm100)));
                        }
                    }
                }
            }

            // A node whose name the layout gives one font for all, "primFontSz for des forName
            // childText op equ", is in the group of that name, and the nodes of a group show
            // their text at one scale of the fit to their boxes.
            if (statesOneFontFor(rDgm, rShape->getInternalName()))
            {
                NamedShapePairs& rDiagramFontHeights
                    = const_cast<SmartArtDiagram&>(rDgm).getDiagramFontHeights();
                rDiagramFontHeights[rShape->getInternalName()].insert({ rShape, {} });
            }

            TextBodyPtr pTextBody = rShape->getTextBody();
            if (!pTextBody || pTextBody->isEmpty())
                break;

            // adjust text size to fit shape
            // deeper paragraphs at the smaller font of their level; runs we sized on a pass
            // before take the font of this pass, the text body is the same on every pass
            if (fFontSize != 0)
            {
                std::set<const TextRun*>& rSizedByLayout(
                    const_cast<SmartArtDiagram&>(rDgm).getRunsSizedByLayout());
                sal_Int32 nShallowest(
                    pTextBody->getParagraphs().front()->getProperties().getLevel());
                for (const auto& aParagraph : pTextBody->getParagraphs())
                    nShallowest = std::min<sal_Int32>(nShallowest,
                                                      aParagraph->getProperties().getLevel());
                const double fSecondPart(
                    secondFontPartOf(rConstraints, getLayoutNode(), rShape->getInternalName()));
                for (auto& aParagraph : pTextBody->getParagraphs())
                {
                    const double fOwnFont(fontOfLevel(
                        fFontSize, aParagraph->getProperties().getLevel() - nShallowest + 1,
                        fSecondPart));
                    for (auto& aRun : aParagraph->getRuns())
                        if (!aRun->getTextCharacterProperties().moHeight.has_value()
                            || rSizedByLayout.count(aRun.get()))
                        {
                            aRun->getTextCharacterProperties().moHeight = fOwnFont * 100;
                            rSizedByLayout.insert(aRun.get());
                        }
                }
            }

            if (!HasCustomText(rDgm, rShape))
            {
                // No customized text properties: enable autofit.
                pTextBody->getTextProperties().maPropertyMap.setProperty(
                    PROP_TextFitToSize, drawing::TextFitToSizeType_AUTOFIT);
            }

            // ECMA-376-1:2016 21.4.7.5 ST_AutoTextRotation (Auto Text Rotation)
            const sal_Int32 nautoTxRot = maMap.count(XML_autoTxRot) ? maMap.find(XML_autoTxRot)->second : XML_upr;
            // the turn the shape is drawn with, its own plus the layout node's (shape rot=270)
            sal_Int32 nShapeRot = rShape->getRotation() + rShape->getDiagramRotation();
            while (nShapeRot < 0)
                nShapeRot += 360 * PER_DEGREE;
            while (nShapeRot > 360 * PER_DEGREE)
                nShapeRot -= 360 * PER_DEGREE;

            switch(nautoTxRot)
            {
                case XML_upr:
                {
                    int n90x = 0;
                    if (nShapeRot >= 315 * PER_DEGREE)
                        /* keep 0 */;
                    else if (nShapeRot > 225 * PER_DEGREE)
                        n90x = -3;
                    else if (nShapeRot >= 135 * PER_DEGREE)
                        n90x = -2;
                    else if (nShapeRot > 45 * PER_DEGREE)
                        n90x = -1;
                    pTextBody->getTextProperties().moTextPreRotation = n90x * 90 * PER_DEGREE;
                    // the shape's own record of the turn, kept where the text body is not
                    rShape->setDiagramTextPreRotation(n90x * 90 * PER_DEGREE);
                }
                break;
                case XML_grav:
                {
                    if (nShapeRot > (90 * PER_DEGREE) && nShapeRot < (270 * PER_DEGREE))
                    {
                        pTextBody->getTextProperties().moTextPreRotation = -180 * PER_DEGREE;
                        rShape->setDiagramTextPreRotation(-180 * PER_DEGREE);
                    }
                    else
                        rShape->setDiagramTextPreRotation(0);
                }
                break;
                case XML_none:
                    rShape->setDiagramTextPreRotation(0);
                break;
            }

            const sal_Int32 atxAnchorVert = maMap.count(XML_txAnchorVert) ? maMap.find(XML_txAnchorVert)->second : XML_mid;

            switch(atxAnchorVert)
            {
                case XML_t:
                pTextBody->getTextProperties().meVA = css::drawing::TextVerticalAdjust_TOP;
                break;
                case XML_b:
                pTextBody->getTextProperties().meVA = css::drawing::TextVerticalAdjust_BOTTOM;
                break;
                case XML_mid:
                // text centered vertically by default
                default:
                pTextBody->getTextProperties().meVA = css::drawing::TextVerticalAdjust_CENTER;
                break;
            }

            pTextBody->getTextProperties().maPropertyMap.setProperty(PROP_TextVerticalAdjust, pTextBody->getTextProperties().meVA);

            // normalize list level
            sal_Int32 nBaseLevel = pTextBody->getParagraphs().front()->getProperties().getLevel();
            for (auto & aParagraph : pTextBody->getParagraphs())
            {
                if (aParagraph->getProperties().getLevel() < nBaseLevel)
                    nBaseLevel = aParagraph->getProperties().getLevel();
            }

            // Start bullets at:
            // 1 - top level
            // 2 - with children (default)
            int nStartBulletsAtLevel = 2;
            ParamMap::const_iterator aBulletLvl = maMap.find(XML_stBulletLvl);
            if (aBulletLvl != maMap.end())
                nStartBulletsAtLevel = aBulletLvl->second;
            nStartBulletsAtLevel--;

            bool isBulletList = false;
            for (auto & aParagraph : pTextBody->getParagraphs())
            {
                sal_Int32 nLevel = aParagraph->getProperties().getLevel() - nBaseLevel;
                aParagraph->getProperties().setLevel(nLevel);
                if (nLevel >= nStartBulletsAtLevel)
                {
                    // the bullet indent goes with the font of the paragraph; a paragraph we
                    // indented on a pass before takes the indent of this pass, the text body is
                    // the same on every pass
                    const double fOwnFont(
                        fFontSize > 0.0 ? fontOfLevel(fFontSize, nLevel + 1,
                                                      secondFontPartOf(rConstraints,
                                                                       getLayoutNode(),
                                                                       rShape->getInternalName()))
                                        : 0.0);
                    const sal_Int64 nIndent(fOwnFont > 0.0 ? bulletIndentFor(fOwnFont)
                                                           : sal_Int64(285750));
                    std::set<const TextParagraph*>& rIndentedByLayout(
                        const_cast<SmartArtDiagram&>(rDgm).getParagraphsIndentedByLayout());
                    const bool bOurs(rIndentedByLayout.count(aParagraph.get()));
                    if (bOurs || !aParagraph->getProperties().getParaLeftMargin().has_value())
                    {
                        sal_Int32 nLeftMargin
                            = o3tl::convert(nIndent * (nLevel - nStartBulletsAtLevel + 1),
                                            o3tl::Length::emu, o3tl::Length::mm100);
                        aParagraph->getProperties().getParaLeftMargin() = nLeftMargin;
                        rIndentedByLayout.insert(aParagraph.get());
                    }

                    if (bOurs || !aParagraph->getProperties().getFirstLineIndentation().has_value())
                        aParagraph->getProperties().getFirstLineIndentation()
                            = o3tl::convert(-nIndent, o3tl::Length::emu, o3tl::Length::mm100);

                    // It is not possible to change the bullet style for text.
                    aParagraph->getProperties().getBulletList().setBulletChar(u"•"_ustr);
                    aParagraph->getProperties().getBulletList().setSuffixNone();
                    isBulletList = true;
                }
            }

            // explicit alignment
            ParamMap::const_iterator aDir = maMap.find(XML_parTxLTRAlign);
            // TODO: XML_parTxRTLAlign
            if (aDir != maMap.end())
            {
                css::style::ParagraphAdjust aAlignment = GetParaAdjust(aDir->second);
                for (auto & aParagraph : pTextBody->getParagraphs())
                    aParagraph->getProperties().setParaAdjust(aAlignment);
            }
            else if (!isBulletList)
            {
                // if not list use default alignment - centered
                for (auto & aParagraph : pTextBody->getParagraphs())
                    aParagraph->getProperties().setParaAdjust(css::style::ParagraphAdjust::ParagraphAdjust_CENTER);
            }
            break;
        }

        default:
            break;
    }

    SAL_INFO(
        "oox.drawingml",
        "Layouting shape " << rShape->getInternalName() << ", alg type: " << mnType << ", ("
        << rShape->getPosition().X << "," << rShape->getPosition().Y << ","
        << rShape->getSize().Width << "," << rShape->getSize().Height << ")");
}

LayoutNode::LayoutNode()
    : LayoutAtom(*this)
    , mnChildOrder(0)
{
}

void LayoutNode::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

bool LayoutNode::setupShape( const SmartArtDiagram& rDgm, const ShapePtr& rShape,
                             const rtl::Reference<svx::diagram::Point>& rPresNode,
                             sal_Int32 nCurrIdx ) const
{
    SAL_INFO(
        "oox.drawingml",
        "Filling content from layout node named \"" << msName
            << "\", modelId \"" << rPresNode->msModelId << "\"");

    // have the presentation node - now, need the actual data node:
    const DiagramData_oox::StringMap::const_iterator aNodeName = rDgm.getData()->getPresOfNameMap().find(
        rPresNode->msModelId);
    if( aNodeName != rDgm.getData()->getPresOfNameMap().end() )
    {
        // Calculate the depth of what is effectively the topmost element.
        sal_Int32 nMinDepth = std::numeric_limits<sal_Int32>::max();
        for (const auto& rPair : aNodeName->second)
        {
            if (rPair.second.mnDepth < nMinDepth)
                nMinDepth = rPair.second.mnDepth;
        }

        for (const auto& rPair : aNodeName->second)
        {
            const DiagramData_oox::SourceIdAndDepth& rItem = rPair.second;
            // rPresNode is the presentation node of the aDataNode2 data node.
            const rtl::Reference<svx::diagram::Point> xDataNode2(
                rDgm.getData()->getPointByModelID(rItem.msSourceId));
            if (!xDataNode2.is())
            {
                //busted, skip it
                continue;
            }

            Shape* pDataNode2Shape(rDgm.getData()->getOrCreateAssociatedShape(*xDataNode2));
            if (nullptr == pDataNode2Shape)
            {
                //busted, skip it
                continue;
            }

            rShape->setDataNodeType(xDataNode2->mnXMLType);

            if (rItem.mnDepth == 0)
            {
                // grab shape attr from topmost element(s)
                rShape->getShapeProperties() = pDataNode2Shape->getShapeProperties();
                rShape->getLineProperties() = pDataNode2Shape->getLineProperties();
                rShape->getFillProperties() = pDataNode2Shape->getFillProperties();
                rShape->getCustomShapeProperties() = pDataNode2Shape->getCustomShapeProperties();
                rShape->setMasterTextListStyle( pDataNode2Shape->getMasterTextListStyle() );

                SAL_INFO(
                    "oox.drawingml",
                    "Custom shape with preset type "
                        << (rShape->getCustomShapeProperties()
                            ->getShapePresetType())
                        << " added for layout node named \"" << msName
                        << "\"");
            }
            else if (rItem.mnDepth == nMinDepth)
            {
                // If no real topmost element, then take properties from the one that's the closest
                // to topmost.
                rShape->getLineProperties() = pDataNode2Shape->getLineProperties();
                rShape->getFillProperties() = pDataNode2Shape->getFillProperties();
            }

            // append text with right outline level
            if( pDataNode2Shape->getTextBody() &&
                !pDataNode2Shape->getTextBody()->getParagraphs().empty() &&
                !pDataNode2Shape->getTextBody()->getParagraphs().front()->getRuns().empty() )
            {
                TextBodyPtr pTextBody=rShape->getTextBody();
                if( !pTextBody )
                {
                    pTextBody = std::make_shared<TextBody>();

                    // also copy text attrs
                    pTextBody->getTextListStyle() =
                        pDataNode2Shape->getTextBody()->getTextListStyle();
                    pTextBody->getTextProperties() =
                        pDataNode2Shape->getTextBody()->getTextProperties();

                    rShape->setTextBody(pTextBody);
                }

                // a shape showing several Points holds all their paragraphs one after the other,
                // with nothing marking where one Point's paragraphs end; destOrd gives the order of
                // the Points, so we need each Point's paragraph count, which only its own text has
                const TextParagraphVector& rSourceParagraphs
                    = pDataNode2Shape->getTextBody()->getParagraphs();
                for (const auto& pSourceParagraph : rSourceParagraphs)
                {
                    TextParagraph& rPara = pTextBody->addParagraph();
                    if (rItem.mnDepth != -1)
                        rPara.getProperties().setLevel(rItem.mnDepth);

                    for (const auto& pRun : pSourceParagraph->getRuns())
                        rPara.addRun(pRun);
                    const TextBodyPtr& rBody = pDataNode2Shape->getTextBody();
                    rPara.getProperties().apply(rBody->getParagraphs().front()->getProperties());
                }
            }
        }
    }
    else
    {
        SAL_INFO(
            "oox.drawingml",
            "ShapeCreationVisitor::visit: no data node name found while"
                " processing shape type "
                << rShape->getCustomShapeProperties()->getShapePresetType()
                << " for layout node named \"" << msName << "\"");
        Shape* pPresNodeShape(rDgm.getData()->getOrCreateAssociatedShape(*rPresNode));
        if (nullptr != pPresNodeShape)
            rShape->getFillProperties().assignUsed(pPresNodeShape->getFillProperties());

        // A layout node with no presOf still stands for the Point its presentation was built
        // for: Picture_Accent_Blocks' sibTrans, a space of 0.0001 between two pictures, is a
        // transition, and the snake tells its transitions from its spaces by that.
        const rtl::Reference<svx::diagram::Point> xAssociated(rDgm.getData()->getPointByModelID(
            rPresNode->getPresentation().msPresentationAssociationId));
        if (xAssociated.is())
            rShape->setDataNodeType(xAssociated->mnXMLType);
    }

    // TODO(Q1): apply styling & coloring - take presentation
    // point's presStyleLbl for both style & color
    // if not found use layout node's styleLbl
    // however, docs are a bit unclear on this
    OUString aStyleLabel = rPresNode->getPresentation().msPresentationLayoutStyleLabel;
    if (aStyleLabel.isEmpty())
        aStyleLabel = msStyleLabel;
    if( !aStyleLabel.isEmpty() )
    {
        const DiagramQStyleMap::const_iterator aStyle = rDgm.getStyles().find(aStyleLabel);
        if( aStyle != rDgm.getStyles().end() )
        {
            const DiagramStyle& rStyle = aStyle->second;
            rShape->getShapeStyleRefs()[XML_fillRef] = rStyle.maFillStyle;
            rShape->getShapeStyleRefs()[XML_lnRef] = rStyle.maLineStyle;
            rShape->getShapeStyleRefs()[XML_effectRef] = rStyle.maEffectStyle;
            rShape->getShapeStyleRefs()[XML_fontRef] = rStyle.maTextStyle;
        }
        else
        {
            SAL_WARN("oox.drawingml", "Style " << aStyleLabel << " not found");
        }

        const DiagramColorMap::const_iterator aColor = rDgm.getColors().find(aStyleLabel);
        if( aColor != rDgm.getColors().end() )
        {
            // A style label is shared by a run of shapes, and a color list is spread
            // over that run. The presentation node states where in the run it sits, so
            // prefer that over the place in the <dgm:forEach> loop that got us here,
            // which is only the same thing when the run happens to be one loop.
            sal_Int32 nColorIdx(rPresNode->getPresentation().mnLayoutStyleIndex);
            sal_Int32 nColorCnt(rPresNode->getPresentation().mnLayoutStyleCount);
            if (nColorIdx < 0 || nColorCnt < 1)
            {
                nColorIdx = nCurrIdx;
                nColorCnt = 0;
            }

            const DiagramColor& rColor=aColor->second;
            if( !rColor.maFillColors.maColors.empty() )
                rShape->getShapeStyleRefs()[XML_fillRef].maPhClr
                    = DiagramColor::getColorByIndex(rColor.maFillColors, nColorIdx, nColorCnt);
            if( !rColor.maLineColors.maColors.empty() )
            {
                const Color aLineColor(
                    DiagramColor::getColorByIndex(rColor.maLineColors, nColorIdx, nColorCnt));
                rShape->getShapeStyleRefs()[XML_lnRef].maPhClr = aLineColor;

                // A line style of a theme carries the width and the dash of a line and leaves the
                // colour to whoever points at it. A shape that defines no line takes the colour
                // that the Diagram gives it as a line, while the width and the dash are from the style.
                // The style names the line of the theme it wants by a number counted from one, and
                // zero is what a shape drawn without any line carries, so leave that one alone.
                LineProperties& rLineProperties(rShape->getLineProperties());

                if (!rLineProperties.maLineFill.moFillType.has_value()
                    && 0 != rShape->getShapeStyleRefs()[XML_lnRef].mnThemedIdx)
                {
                    rLineProperties.maLineFill.moFillType = XML_solidFill;
                    rLineProperties.maLineFill.maFillColor = aLineColor;
                }
            }
            if( !rColor.maEffectColors.maColors.empty() )
                rShape->getShapeStyleRefs()[XML_effectRef].maPhClr
                    = DiagramColor::getColorByIndex(rColor.maEffectColors, nColorIdx, nColorCnt);
            if( !rColor.maTextFillColors.maColors.empty() )
                rShape->getShapeStyleRefs()[XML_fontRef].maPhClr
                    = DiagramColor::getColorByIndex(rColor.maTextFillColors, nColorIdx, nColorCnt);
        }
    }

    // even if no data node found, successful anyway. it's
    // contained at the layoutnode
    return true;
}

const LayoutNode* LayoutNode::getParentLayoutNode() const
{
    for (LayoutAtomPtr pAtom = getParent(); pAtom; pAtom = pAtom->getParent())
    {
        auto pLayoutNode = dynamic_cast<LayoutNode*>(pAtom.get());
        if (pLayoutNode)
            return pLayoutNode;
    }

    return nullptr;
}

void ShapeAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
