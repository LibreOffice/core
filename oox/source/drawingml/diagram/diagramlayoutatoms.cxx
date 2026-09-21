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

#include "diagramlayoutatoms.hxx"

#include <cmath>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
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
void SnakeAlg::layoutShapeChildren(const AlgAtom& rAlg, const ShapePtr& rShape,
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
            if (bSpace && !rChild->getDataNodeType() && oFactor)
            {
                oGapOfCell = *oFactor;
                bGapTrails = rChild == rShape->getChildren().back();
            }
            else if (!bSpace && !oNodeFactor)
                oNodeFactor = oFactor;
        }
        if (oGapOfCell)
            *oGapOfCell /= oNodeFactor.value_or(1.0);
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
    bool bHorizontal = true;
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
            bHorizontal = false;
            break;
        case XML_bR:
            nIncX = -1;
            nIncY = -1;
            bHorizontal = false;
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
    double fAspectRatio = 0.54; // diagram should not spill outside, earlier it was 0.6

    // A layout can say itself where the flow breaks into the next line instead of leaving
    // that to the fit. With bkpt fixed the file names how many shapes go into one line and
    // bkPtFixedVal carries that number, and flowDir says whether a line is a row or a column.
    const sal_Int32 nBreak = rMap.count(XML_bkpt) ? rMap.find(XML_bkpt)->second : XML_endCnv;
    const sal_Int32 nBreakAt
        = rMap.count(XML_bkPtFixedVal) ? rMap.find(XML_bkPtFixedVal)->second : 0;
    const sal_Int32 nFlowDir = rMap.count(XML_flowDir) ? rMap.find(XML_flowDir)->second : XML_row;

    sal_Int32 nCol = 1;
    sal_Int32 nRow = 1;
    sal_Int32 nMaxRowWidth = 0;
    const bool bTransitionsAmongChildren(std::any_of(
        rShape->getChildren().begin(), rShape->getChildren().end(),
        [](const ShapePtr& rChild) { return rChild->getDataNodeType() == XML_sibTrans; }));
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
            const double fAcross(nTry + (nTry - 1) * fSpace);
            const double fDown((nRows + (nRows - 1) * fSpace) / fChildAspectRatio);
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
    std::optional<sal_Int32> oCellWidth;
    std::optional<sal_Int32> oCellHeight;
    for (const Constraint& rConstraint : rConstraints)
    {
        if (rConstraint.mnFor != XML_ch || !rConstraint.msForName.isEmpty()
            || rConstraint.mnPointType != XML_all || !rConstraint.msRefForName.isEmpty()
            || rConstraint.mfValue != 0.0)
            continue;
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
        nCol = std::clamp<sal_Int32>((rShape->getSize().Width + fSpace * *oCellWidth)
                                         / (*oCellWidth * (1.0 + fSpace)),
                                     1, nCount);
        nRow = std::ceil(static_cast<double>(nCount) / nCol);
    }

    SAL_INFO("oox.drawingml", "Snake layout grid: " << nCol << "x" << nRow);

    // The gaps of a row: one less than the cells, or as many when a space trails the last cell.
    const sal_Int32 nGaps = bGapTrails ? nCol : nCol - 1;
    sal_Int32 nWidth = oCellWidth ? *oCellWidth : rShape->getSize().Width / (nCol + nGaps * fSpace);
    awt::Size aChildSize(nWidth, oCellHeight ? *oCellHeight : nWidth * fAspectRatio);

    // A snake in the offset mode, "off val=off", steps every row aside by a part of a cell, a
    // staircase: the second row stands alignOff of a cell further along than the first, the
    // third twice that. alignOff is stated on the snake itself, a bare value that is the part.
    const bool bOffsetMode((rMap.count(XML_off) ? rMap.find(XML_off)->second : XML_ctr)
                           == XML_off);
    // Where the layout states no alignOff the rows step aside by a whole cell: the blocks of
    // Picture_Accent_Blocks stand two and two, the upper row one cell to the right of the lower.
    double fAlignOff(0.0);
    if (bOffsetMode)
    {
        fAlignOff = 1.0;
        for (const Constraint& rConstraint : rConstraints)
            if (rConstraint.mnType == XML_alignOff && rConstraint.mfValue != 0.0)
                fAlignOff = rConstraint.mfValue;
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
        sal_Int32 nHeight = rShape->getSize().Height / (nRow + (nRow + nNumSpaces) * fSpace);

        if (fChildAspectRatio > 1)
        {
            // Shrink width if the aspect ratio requires it.
            nWidth = std::min(rShape->getSize().Width,
                              static_cast<sal_Int32>(nHeight * fChildAspectRatio));
            aChildSize = awt::Size(nWidth, nHeight);
        }


        bHorizontal = false;
    }

    // The staircase has to fit the room with its steps: a cell is at most the width over the
    // cells of a row with their gaps and the offsets of the rows below the first. A cell whose
    // height came from its width shrinks in step with it.
    if (fAlignOff > 0.0 && nRow > 1)
    {
        const sal_Int32 nRoom(static_cast<sal_Int32>(
            rShape->getSize().Width / (nCol + nGaps * fSpace + (nRow - 1) * fAlignOff)));
        if (nWidth > nRoom && nRoom > 0)
        {
            if (!(nCol == 1) && !oCellHeight)
                aChildSize.Height = static_cast<sal_Int32>(
                    static_cast<double>(aChildSize.Height) * nRoom / nWidth);
            nWidth = nRoom;
            aChildSize.Width = nWidth;
        }
    }

    awt::Point aCurrPos(0, 0);
    if (nIncX == -1)
        aCurrPos.X = rShape->getSize().Width - aChildSize.Width;
    if (nIncY == -1)
        aCurrPos.Y = rShape->getSize().Height - aChildSize.Height;
    else if (bSpaceFromConstraints)
    {
        if (!bHorizontal)
        {
            // Initial vertical offset to have upper spacing (outside, so double amount).
            aCurrPos.Y = aChildSize.Height * fSpace * 2;
        }
    }

    sal_Int32 nStartX = aCurrPos.X;
    sal_Int32 nColIdx = 0, index = 0;

    const sal_Int32 aContDir
        = rMap.count(XML_contDir) ? rMap.find(XML_contDir)->second : XML_sameDir;

    switch (aContDir)
    {
        case XML_sameDir:
        {
            sal_Int32 nRowHeight = 0;
            for (auto& aCurrShape : rShape->getChildren())
            {
                aCurrShape->setPosition(aCurrPos);
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
                    aCurrSize.Width = rShape->getSize().Width * fWidthFactor;
                }
                if (fChildAspectRatio)
                {
                    aCurrSize.Height = aCurrSize.Width / fChildAspectRatio;

                    // Child shapes are not allowed to leave their parent.
                    aCurrSize.Height = std::min<sal_Int32>(
                        aCurrSize.Height, rShape->getSize().Height / (nRow + (nRow - 1) * fSpace));
                }
                if (aCurrSize.Height > nRowHeight)
                {
                    nRowHeight = aCurrSize.Height;
                }
                aCurrShape->setSize(aCurrSize);
                aCurrShape->setChildSize(aCurrSize);

                index++; // counts index of child, helpful for positioning.

                if (index % nCol == 0 || ((index / nCol) + 1) != nRow)
                    aCurrPos.X += nIncX * (aCurrSize.Width + fSpace * aCurrSize.Width);

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
                                        : aChildSize.Width);
                                fExtent += fWidth * (i + 1 < nTo ? 1.0 + fSpace : 1.0);
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
                    aCurrPos.Y += nIncY * (nRowHeight + fSpace * nRowHeight);
                    nColIdx = 0;
                    nRowHeight = 0;
                }

                // positions children in the last row. Every cell of it steps on by its width
                // and the gap, also when the last row is the only one and holds no more than
                // three cells, as the calendars' single row of four days.
                if (index % nCol != 0 && ((index / nCol) + 1) == nRow)
                    aCurrPos.X += (nIncX * (aCurrSize.Width + fSpace * aCurrSize.Width));
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
                    aCurrPos.X += (aChildSize.Width + fSpace * aChildSize.Width);
                else if (index % nCol != 0
                         && ((index / nCol) + 1) != nRow) // child other than placed at last column
                    aCurrPos.X -= (aChildSize.Width + fSpace * aChildSize.Width);

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
                                     + (nIncX * (aChildSize.Width + fSpace * aChildSize.Width)) / 2;
                    else if (((index / nCol) + 1) % 2 != 0)
                        aCurrPos.X = nStartX;

                    aCurrPos.Y += nIncY * (aChildSize.Height + fSpace * aChildSize.Height);
                    nColIdx = 0;
                }

                // positions children in the last row.
                if (index % nCol != 0 && index >= 3 && ((index / nCol) + 1) == nRow
                    && ((index / nCol) + 1) % 2 == 0)
                    //if row%2=0 then start from left else
                    aCurrPos.X -= (nIncX * (aChildSize.Width + fSpace * aChildSize.Width));
                else if (index % nCol != 0 && index >= 3 && ((index / nCol) + 1) == nRow
                         && ((index / nCol) + 1) % 2 != 0)
                    // start from right
                    aCurrPos.X += (nIncX * (aChildSize.Width + fSpace * aChildSize.Width));
            }
            break;
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

void PyraAlg::layoutShapeChildren(const ShapePtr& rShape)
{
    if (rShape->getChildren().empty() || rShape->getSize().Width == 0
        || rShape->getSize().Height == 0)
        return;

    // const sal_Int32 nDir = maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromT;
    // const sal_Int32 npyraAcctPos = maMap.count(XML_pyraAcctPos) ? maMap.find(XML_pyraAcctPos)->second : XML_bef;
    // const sal_Int32 ntxDir = maMap.count(XML_txDir) ? maMap.find(XML_txDir)->second : XML_fromT;
    // const sal_Int32 npyraLvlNode = maMap.count(XML_pyraLvlNode) ? maMap.find(XML_pyraLvlNode)->second : XML_level;
    // uncomment when use in code.

    sal_Int32 nCount = rShape->getChildren().size();
    double fAspectRatio = 0.32;

    awt::Size aChildSize = rShape->getSize();
    aChildSize.Width /= nCount;
    aChildSize.Height /= nCount;

    awt::Point aCurrPos(0, 0);
    aCurrPos.X = fAspectRatio * aChildSize.Width * (nCount - 1);
    aCurrPos.Y = fAspectRatio * aChildSize.Height;

    for (auto& aCurrShape : rShape->getChildren())
    {
        aCurrShape->setPosition(aCurrPos);
        if (nCount > 1)
        {
            aCurrPos.X -= aChildSize.Height / (nCount - 1);
        }
        aChildSize.Width += aChildSize.Height;
        aCurrShape->setSize(aChildSize);
        aCurrShape->setChildSize(aChildSize);
        aCurrPos.Y += (aChildSize.Height);
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
// size is taken. Nothing where none is laid out yet.
static std::optional<sal_Int32> sizeOfLaidOutShape(const SmartArtDiagram& rDgm,
                                                   std::u16string_view rName, sal_Int32 nWhat)
{
    for (const auto& rEntry : rDgm.getLayout()->getPresPointShapeMap())
    {
        const ShapePtr& pShape(rEntry.second);
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

    const LayoutProperty::const_iterator aRefType = aRef->second.find(rConstraint.mnRefType);
    sal_Int32 nInferredValue = 0;
    if (aRefType != aRef->second.end())
    {
        // Reference is found directly.
        const sal_Int32 nValue(aRefType->second * rConstraint.mfFactor);
        rProperties[rConstraint.msForName][rConstraint.mnType] = nValue;
        if (isUserVariable(rConstraint.mnType))
            rUserVariables[rConstraint.mnType] = nValue;
    }
    else if (inferFromLayoutProperty(aRef->second, rConstraint.mnRefType, nInferredValue))
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
        // EMU.
        const double fValue = o3tl::convert(rConstraint.mfValue,
                                            isFontUnit(rConstraint.mnRefType) ? o3tl::Length::pt
                                                                              : o3tl::Length::mm,
                                            o3tl::Length::emu);
        rProperties[rConstraint.msForName][rConstraint.mnType] = fValue;
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
    if (bAtTheTop && rAlg.getAspectRatio() > 0.0)
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

    for (auto& aCurrShape : rShape->getChildren())
    {
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
        for (size_t nPass = 0; nPass < 8; ++nPass)
        {
            bool bMoved(false);
            for (const Constraint& rBound : aBounds)
            {
                if (rBound.msForName != aCurrShape->getInternalName())
                    continue;
                const auto aOwn = aProperties.find(rBound.msForName);
                if (aOwn == aProperties.end() || !aOwn->second.count(rBound.mnType))
                    continue;
                std::optional<sal_Int32> oLimit;
                if (rBound.mnRefType == XML_none && rBound.mfValue != 0.0)
                    oLimit = static_cast<sal_Int32>(o3tl::convert(
                        rBound.mfValue, isFontUnit(rBound.mnType) ? o3tl::Length::pt
                                                                  : o3tl::Length::mm,
                        o3tl::Length::emu));
                else
                {
                    const auto aRef = aProperties.find(rBound.msRefForName);
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
                else
                    rValue = std::max(rValue, *oLimit);
                bMoved = bMoved || rValue != nBefore;
            }
            if (!bMoved)
                break;
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

                Constraint aConstraint(rConstraint);
                aConstraint.msForName = pLayoutNode->getName();
                aConstraint.msRefForName = pLayoutNode->getName();

                applyConstraintToLayout(rDgm, rShape, rConstraints, aConstraint, aProperties,
                                    const_cast<SmartArtDiagram&>(rDgm).getUserVariables());
            }
        }

        awt::Size aSize = rShape->getSize();
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

            if ((it = rProp.find(XML_w)) != rProp.end())
                aSize.Width = std::min(it->second, rShape->getSize().Width);
            if ((it = rProp.find(XML_h)) != rProp.end())
                aSize.Height = std::min(it->second, rShape->getSize().Height);

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

            if ((it = rProp.find(XML_l)) != rProp.end() && (it2 = rProp.find(XML_r)) != rProp.end())
                aSize.Width = it2->second - it->second;
            if ((it = rProp.find(XML_t)) != rProp.end() && (it2 = rProp.find(XML_b)) != rProp.end())
                aSize.Height = it2->second - it->second;

            aPos.X += nParentXOffset;
            aPos.Y += nParentYOffset;
            aSize.Width = std::min(aSize.Width, rShape->getSize().Width - aPos.X);
            aSize.Height = std::min(aSize.Height, rShape->getSize().Height - aPos.Y);

            // A connector that states its diam is an arc of a circle of that diameter, its band
            // as thick as its h, so its box is the diameter and the thickness. Its l and t, or
            // ctrX and ctrY, say where the middle of the circle stands, not a corner; where it
            // states none, the circle is about the shape its diam refers to, the gear a
            // connector of 1.1 of the gear's width runs round. Segmented_Cycle's arrow wedges run
            // round its wedges on a circle 0.84 of the composite across, from the middle of it.
            if ((it = rProp.find(XML_diam)) != rProp.end() && it->second > 0
                && aCurrShape->getSubType() == XML_conn)
            {
                const sal_Int32 nDiameter(it->second);
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
            SAL_WARN("oox.drawingml", "composite layout properties not found for shape "
                                          << aCurrShape->getInternalName());

        aCurrShape->setSize(aSize);
        aCurrShape->setChildSize(aSize);
        aCurrShape->setPosition(aPos);

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

    // See if all vertical space is used or we have to center the content. A box of the
    // composite's own shape stands in the middle of the room already.
    if (nParentYOffset > 0)
        return;
    if (!(nVertMin >= 0 && nVertMin <= nVertMax && nVertMax <= rParent[XML_h]))
        return;

    sal_Int32 nDiff = rParent[XML_h] - (nVertMax - nVertMin);
    if (nDiff > 0)
    {
        for (auto& aCurrShape : rShape->getChildren())
        {
            awt::Point aPosition = aCurrShape->getPosition();
            aPosition.Y += nDiff / 2;
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
 * The parent of rFrom in the data tree, and the place rFrom takes among its children.
 */
OUString parentOf(const SmartArtDiagram& rDgm, std::u16string_view rFrom, sal_Int32& rOrder)
{
    for (const rtl::Reference<svx::diagram::Connection>& rConnection :
             rDgm.getData()->getConnections())
        if (rConnection->mnXMLType == svx::diagram::TypeConstant::XML_parOf
            && rConnection->msDestId == rFrom)
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
            const std::vector<OUString> aNodes(childrenOf(rDgm, aParent, XML_all));
            const std::vector<OUString> aWanted(childrenOf(rDgm, aParent, nWanted));
            for (size_t nPlace = 0; nPlace < aNodes.size() && nPlace < aWanted.size(); ++nPlace)
            {
                sal_Int32 nSiblingOrder(0);
                parentOf(rDgm, aNodes[nPlace], nSiblingOrder);

                const bool bKeep(nAxis == XML_followSib
                                     ? (bTransition ? nSiblingOrder >= nOrder
                                                    : nSiblingOrder > nOrder)
                                     : (bTransition ? nSiblingOrder <= nOrder
                                                    : nSiblingOrder < nOrder));
                if (bKeep)
                    aOut.push_back(aWanted[nPlace]);
            }
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
void gatherLayoutNodes(const LayoutAtom& rAtom, std::map<OUString, const LayoutNode*>& rOut)
{
    for (const LayoutAtomPtr& pChild : rAtom.getChildren())
    {
        if (const LayoutNode* pNode = dynamic_cast<const LayoutNode*>(pChild.get()))
            rOut[pNode->getName()] = pNode;
        gatherLayoutNodes(*pChild, rOut);
    }
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

void settleNamedConnectors(const SmartArtDiagram& rDgm)
{
    const PresPointShapeMap& rShapes(rDgm.getLayout()->getPresPointShapeMap());

    std::map<OUString, const LayoutNode*> aNodes;
    if (rDgm.getLayout()->getNode())
    {
        aNodes[rDgm.getLayout()->getNode()->getName()] = rDgm.getLayout()->getNode().get();
        gatherLayoutNodes(*rDgm.getLayout()->getNode(), aNodes);
    }

    for (const auto& rEntry : rShapes)
    {
        const rtl::Reference<svx::diagram::Point>& xOwn(rEntry.first);
        const ShapePtr& pShape(rEntry.second);
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
        if (nRoute == XML_bend)
            continue;

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

        // The flow runs the way the two lie apart, and this is for a connector between two that
        // stand in one line. One that goes round a corner is left alone.
        awt::Point aOwnPos(pShape->getPosition());
        const awt::Size& rOwnSize(pShape->getSize());
        if (nAcrossX >= nAcrossY)
        {
            if (nAcrossY * 4 > std::min(rSourceSize.Height, rTargetSize.Height))
                continue;
            aOwnPos.Y = (aSourceMiddle.Y + aTargetMiddle.Y) / 2 - aParentAt->Y
                        - rOwnSize.Height / 2;
        }
        else
        {
            if (nAcrossX * 4 > std::min(rSourceSize.Width, rTargetSize.Width))
                continue;
            aOwnPos.X = (aSourceMiddle.X + aTargetMiddle.X) / 2 - aParentAt->X
                        - rOwnSize.Width / 2;
        }
        pShape->setPosition(aOwnPos);
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
    OUString sParentId;
    sal_Int32 nOrder(0);
    for (const rtl::Reference<svx::diagram::Connection>& aCxn : rDgm.getData()->getConnections())
    {
        if (aCxn->mnXMLType != svx::diagram::TypeConstant::XML_parOf)
            continue;

        if (aCxn->msDestId == sNodeId || aCxn->msSibTransId == sNodeId
            || aCxn->msParTransId == sNodeId)
        {
            sParentId = aCxn->msSourceId;
            nOrder = aCxn->mnSourceOrder;
            break;
        }
    }

    if (sParentId.isEmpty())
        return;

    for (const rtl::Reference<svx::diagram::Connection>& aCxn : rDgm.getData()->getConnections())
        if (aCxn->mnXMLType == svx::diagram::TypeConstant::XML_parOf
            && aCxn->msSourceId == sParentId)
            rSiblings++;

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
            return compareResult(maCond.mnOp,
                                     rPresPoint->getLayoutVariables().mnDirection,
                                     maCond.mnVal);
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
/// Does the first data node of this shape have customized text properties?
bool HasCustomText(const SmartArtDiagram& rDgm, const ShapePtr& rShape)
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

    // If we have a data node, see if its text is customized or not.
    if (xDataNode.is())
    {
        return xDataNode->mbCustomText;
    }

    return false;
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

        const double fFactor(readHeightOfOwnWidthFactor(aOwn));
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

/// The extent each child of a linear layout asks for along the axis the children are placed on,
/// as a multiple of the parent extent. A child states its wish as a factor of a reference: the
/// parent, or an other child named directly or addressed by its data point type. A reference may
/// point at a child whose own wish is not worked out yet, so the wishes are collected in several
/// passes, until a pass adds nothing new.
class LinearChildExtents
{
public:
    /// nAxisType is XML_w or XML_h, nParentExtent the parent extent along that axis, in EMU.
    void read(const SmartArtDiagram& rDgm, const ShapePtr& rRow,
              const std::vector<Constraint>& rConstraints, sal_Int32 nAxisType,
              sal_Int32 nParentExtent);

    /// True when at least one constraint stated an extent.
    bool isFilled() const { return mbFilled; }

    /// True when a wish walks back at least half of the largest wish, so that what follows lies
    /// over a child rather than a little closer to it.
    bool hasOverlap() const { return mbOverlap; }

    /// The extent rShape asks for, as a multiple of the parent extent, or nothing when no
    /// constraint mentions that child.
    std::optional<double> get(const oox::drawingml::Shape& rShape) const;

    /// States the extent of the child named rName outright, as a part of the parent's extent.
    void set(const OUString& rName, double fFactor)
    {
        maByName[rName] = fFactor;
        mbFilled = true;
    }

private:
    bool lookup(const OUString& rName, sal_Int32 nPointType, double& rFactor) const;

    std::map<OUString, double> maByName;
    std::map<sal_Int32, double> maByPointType;
    /// The extents stated from here for the nodes below the row's children, for des, as parts of
    /// the row's extent. No child takes one; a child's extent that refers to such a name reads it.
    std::map<OUString, double> maBelowByName;
    bool mbFilled = false;
    bool mbOverlap = false;
};

bool LinearChildExtents::lookup(const OUString& rName, sal_Int32 nPointType, double& rFactor) const
{
    if (!rName.isEmpty())
    {
        const auto aIt = maByName.find(rName);
        if (aIt != maByName.end())
        {
            rFactor = aIt->second;
            return true;
        }
        const auto aBelow = maBelowByName.find(rName);
        if (aBelow == maBelowByName.end())
            return false;
        rFactor = aBelow->second;
        return true;
    }

    auto aIt = maByPointType.find(nPointType);
    if (aIt == maByPointType.end())
        return false;
    rFactor = aIt->second;
    return true;
}

void LinearChildExtents::read(const SmartArtDiagram& rDgm, const ShapePtr& rRow,
                              const std::vector<Constraint>& rConstraints, sal_Int32 nAxisType,
                              sal_Int32 nParentExtent)
{
    if (nParentExtent <= 0)
        return;
    const auto aIsChild = [&rRow](std::u16string_view rName) {
        for (const ShapePtr& pChild : rRow->getChildren())
            if (pChild->getInternalName() == rName)
                return true;
        return false;
    };
    const sal_Int32 nOffsetType(nAxisType == XML_h ? XML_hOff : XML_wOff);

    size_t nKnown = 0;
    for (size_t nPass = 0; nPass < 8; ++nPass)
    {
        for (const Constraint& rConstraint : rConstraints)
        {
            if (rConstraint.mnType != nAxisType)
                continue;

            // A constraint for the children of this row, or one stated levels above for every
            // node below by name, for des, where the name is a child of this row. One for des
            // that names no child names a node further below: no child takes it, but a child
            // whose extent refers to that name reads it, the width of a spacer stated as the
            // width of the connector inside it say.
            if (rConstraint.mnFor != XML_ch && rConstraint.mnFor != XML_des)
                continue;
            const bool bBelow(rConstraint.mnFor == XML_des && !aIsChild(rConstraint.msForName));

            // An empty name and the catch-all point type together address no single child.
            const bool bByName = !rConstraint.msForName.isEmpty();
            if (!bByName && rConstraint.mnPointType == XML_all)
                continue;
            if (bBelow && !bByName)
                continue;

            double fFactor = 0.0;
            if (rConstraint.mnRefType == nAxisType)
            {
                double fReference = 1.0;
                if (rConstraint.mnRefFor == XML_ch || !rConstraint.msRefForName.isEmpty())
                {
                    if (!lookup(rConstraint.msRefForName, rConstraint.mnRefPointType, fReference))
                    {
                        // A shape that is no child of this row, the picture beside it say, is
                        // laid out already where the node above placed it before the row, so
                        // its size can be read off it, as a part of the row's extent.
                        const std::optional<sal_Int32> aOf(
                            rConstraint.msRefForName.isEmpty()
                                ? std::nullopt
                                : sizeOfLaidOutShape(rDgm, rConstraint.msRefForName,
                                                     nAxisType));
                        if (!aOf)
                            continue;
                        fReference = static_cast<double>(*aOf) / nParentExtent;
                    }
                }
                fFactor = fReference * rConstraint.mfFactor;
            }
            else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue != 0.0
                     && std::isfinite(rConstraint.mfValue))
            {
                // A bare value is stated in mm, while a shape extent is always in EMU. A layout
                // may also ask for an endless extent, which states no size at all.
                fFactor = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm, o3tl::Length::emu)
                          / static_cast<double>(nParentExtent);
            }
            else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue == 0.0
                     && rConstraint.mnOperator == XML_none)
            {
                // A constraint that refers to nothing and states no value is an extent of
                // nothing: "h for des forName=thickLine" makes the line a line, no height at all,
                // and the row gives the child no room along the axis. One with "op=equ" and
                // nothing else says the children are the same size, not that they have none.
                if (bBelow)
                    maBelowByName[rConstraint.msForName] = 0.0;
                else if (bByName)
                    maByName[rConstraint.msForName] = 0.0;
                else
                    maByPointType[rConstraint.mnPointType] = 0.0;
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
                          / static_cast<double>(nParentExtent);
            }

            // A wish of no size at all says nothing about the child.
            if (fFactor == 0.0 || !std::isfinite(fFactor))
                continue;

            if (bBelow)
                maBelowByName[rConstraint.msForName] = fFactor;
            else if (bByName)
                maByName[rConstraint.msForName] = fFactor;
            else
                maByPointType[rConstraint.mnPointType] = fFactor;
        }

        const size_t nNow = maByName.size() + maByPointType.size() + maBelowByName.size();
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

    // A wish of less than nothing that could not be worked out, one stated as a part of the
    // other axis say, is not known to be small, so it counts as an overlap as well.
    for (const Constraint& rConstraint : rConstraints)
        if (rConstraint.mnType == nAxisType && rConstraint.mfFactor < 0.0
            && !rConstraint.msForName.isEmpty()
            && !maByName.count(rConstraint.msForName)
            && !maBelowByName.count(rConstraint.msForName))
            mbOverlap = true;
}

std::optional<double> LinearChildExtents::get(const oox::drawingml::Shape& rShape) const
{
    // A name is the more exact way to address a child, so it wins over the data point type.
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
    const sal_Int32 nBreakAt(aParam(XML_bkPtFixedVal, 0));
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

    // the connectors of the branch, one before each row, from the node to that row's node
    const sal_Int32 nFromX(bLeft ? -nGap : aBranch.Width + nGap);
    const sal_Int32 nNodeMiddleY(nNodeTop + aNode.Height / 2);
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

    double fWidth(0.0), fHeight(0.0);
    const double fBetweenRoots(
        readSidewaysGap(rConstraints, XML_sibSp, aKnown, aRows.front().aNode.Height, 0.15));
    for (size_t nRow = 0; nRow < aRows.size(); ++nRow)
    {
        fWidth = std::max(fWidth, aRows[nRow].fWidth);
        fHeight += aRows[nRow].fHeight + (nRow > 0 ? fBetweenRoots : 0.0);
    }
    if (fWidth <= 0.0 || fHeight <= 0.0)
        return false;
    const double fScale(
        std::min(rShape->getSize().Width / fWidth, rShape->getSize().Height / fHeight));

    SmartArtDiagram& rMutable(const_cast<SmartArtDiagram&>(rDgm));
    rMutable.getLaidOutSideways().insert(rShape.get());
    double fY((rShape->getSize().Height - fHeight * fScale) / 2.0);
    for (size_t nRow = 0; nRow < aRows.size(); ++nRow)
    {
        const SidewaysRoot& rRow(aRows[nRow]);
        const sal_Int32 nRowWidth(static_cast<sal_Int32>(rRow.fWidth * fScale));
        const sal_Int32 nRowLeft(bLeft ? 0 : rShape->getSize().Width - nRowWidth);
        placeSidewaysRoot(rMutable, rRow, awt::Point(nRowLeft, static_cast<sal_Int32>(fY)), fScale,
                          bLeft);
        fY += rRow.fHeight * fScale + fBetweenRoots * fScale;
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
        pChild->setSubType(XML_bentConnector3);
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
    const double fLevelGap(readSidewaysGap(rConstraints, XML_sp, aKnown, fLevelHeight, 0.25));

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
            }

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

            // A line between the shapes states how thick it is, h with a val in millimetres.
            std::map<OUString, double> aStatedHeight;
            gatherStatedHeights(mrLayoutNode, aStatedHeight);

            // Where a spoke starts and ends, and how much of its way it leaves free at the start.
            std::map<OUString, std::pair<sal_Int32, sal_Int32>> aEndSites;
            gatherEndSites(mrLayoutNode, aEndSites);
            std::map<OUString, double> aBeginPads;
            gatherBeginPads(mrLayoutNode, aBeginPads);

            // A layout can state the width of one child as a part of the width of another,
            // the node as 1.5 of the middle shape say, or 0.7 of it. Those parts are kept, by
            // name, against the width every child would have on its own, and a chain of them
            // is followed as far as it goes.
            std::map<OUString, double> aWidthOf;
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

            const auto aSpansTheCircle = [&](const oox::drawingml::ShapePtr& rCycleChild) {
                return aSpanningNames.count(rCycleChild->getInternalName()) > 0
                       || (bSpanningTransitions && rCycleChild->getSubType() == XML_conn);
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
            const auto aLargestChild = [&]() {
                sal_Int32 nRingChildWidth(nPlainWidth);
                sal_Int32 nRingChildHeight(nPlainHeight);
                for (const auto& rCycleChild : rShape->getChildren())
                    if (rCycleChild->getSubType() != XML_conn)
                    {
                        const awt::Size aOwn(aSizeOfChild(rCycleChild->getInternalName(),
                                                          nPlainWidth, nPlainHeight));
                        nRingChildWidth = std::max(nRingChildWidth, aOwn.Width);
                        nRingChildHeight = std::max(nRingChildHeight, aOwn.Height);
                    }
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

            // With a node in the middle and sp stated, the ring is not fitted to the room: its
            // radius is the middle shape's edge, then sp, then the node's edge, along the spoke,
            // and it is the shapes that are scaled until the whole stands in the room. sp is a
            // part of a named child's width and scales along, or a length in millimetres, which
            // does not, so the scale is found in a few rounds. An ellipse reaches as far as its
            // half width whichever way the spoke runs, a box as far as its side that way.
            // The edge of a shape along a spoke at an angle: an ellipse reaches as far as its
            // half width whichever way, a box as far as the side the spoke runs into, and at
            // most to its corner.
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
                    if (rConstraint.mnRefType == XML_w && !rConstraint.msRefForName.isEmpty()
                        && rConstraint.mfFactor > 0.0)
                    {
                        fSpacePart = rConstraint.mfFactor;
                        aSpaceOf = rConstraint.msRefForName;
                    }
                    else if (rConstraint.mnRefType == XML_none && rConstraint.mfValue > 0.0)
                        fSpaceFixed = o3tl::convert(rConstraint.mfValue, o3tl::Length::mm,
                                                    o3tl::Length::emu);
                }

                if (fSpacePart > 0.0 || fSpaceFixed > 0.0)
                {
                    const oox::drawingml::ShapePtr& rMiddle(aCycleChildren.front());

                    // The ring for a scale of the plain sizes, and the box the middle shape and
                    // the nodes on that ring cover, against the middle of the ring.
                    struct RingAndCover
                    {
                        double fRing;
                        double fLeft, fTop, fRight, fBottom;
                    };
                    const auto aRingFor = [&](double fScale) {
                        const sal_Int32 nWidth(nPlainWidth * fScale);
                        const sal_Int32 nHeight(nPlainHeight * fScale);
                        const awt::Size aMiddle(
                            aSizeOfChild(rMiddle->getInternalName(), nWidth, nHeight));
                        double fSpace(fSpaceFixed);
                        if (fSpacePart > 0.0)
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
                            aOut.fLeft = std::min(aOut.fLeft, fX - aOwn.Width / 2.0);
                            aOut.fRight = std::max(aOut.fRight, fX + aOwn.Width / 2.0);
                            aOut.fTop = std::min(aOut.fTop, fY - aOwn.Height / 2.0);
                            aOut.fBottom = std::max(aOut.fBottom, fY + aOwn.Height / 2.0);
                            ++nPlace;
                        }
                        return aOut;
                    };

                    double fScale(1.0);
                    RingAndCover aRing(aRingFor(fScale));
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
                        const double fPad(aPad != aBeginPads.end() ? aPad->second * fWhole : 0.0);
                        const double fWay(std::max(1.0, fWhole - fPad));
                        const double fMiddle(fCentreEdge + fPad + fWay / 2);
                        // As thick as the line says it is, or as the ring says for it, h as a
                        // part of the width of a named shape on the ring, the middle one for a
                        // fat arrow, or a twelfth of the room like any other connector of the
                        // ring where nothing says anything.
                        const auto aThick = aStatedHeight.find(aCurrShape->getInternalName());
                        sal_Int32 nThick(
                            aThick != aStatedHeight.end()
                                ? static_cast<sal_Int32>(aThick->second)
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
                        const awt::Size aSpokeSize(static_cast<sal_Int32>(fWay), nThick);
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
                        aCurrSize = awt::Size(nSpan, nSpan);
                        nCurrRadius = 0;
                    }
                    else if (aCurrShape->getSubType() == XML_conn)
                    {
                        aCurrSize = aSizeOfChild(aCurrShape->getInternalName(),
                                                 aConnectorSize.Width, aConnectorSize.Height);
                        nCurrRadius = nConnectorRadius;
                    }
                    const awt::Point aCurrPos(
                        aRingCentre.Width + nCurrRadius * sin(basegfx::deg2rad(fAngle))
                            - aCurrSize.Width / 2,
                        aRingCentre.Height - nCurrRadius * cos(basegfx::deg2rad(fAngle))
                            - aCurrSize.Height / 2);

                    aCurrShape->setPosition(aCurrPos);
                    aCurrShape->setSize(aCurrSize);
                    aCurrShape->setChildSize(aCurrSize);

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
            const double fSpaceHeight = readSpacingFactor(rConstraints, XML_h, 0.3);

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
            // one row
            const double fHeightOfWidth = readHeightOfOwnWidthFactor(rConstraints);
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
            if (mnType == XML_hierChild)
            {
                std::map<OUString, const LayoutNode*> aLayoutNodes;
                gatherLayoutNodes(mrLayoutNode, aLayoutNodes);
                for (const ShapePtr& pChild : rShape->getChildren())
                {
                    if (pChild->getSubType() != XML_conn)
                        continue;
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

            if (mnType == XML_hierChild && nHorizontalShapesCount == 1 && !bStraightConnectors)
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

                        // A bent connector defines that elbow once its corner sits on its own left
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
                if (rConstraint.mnType == XML_w)
                {
                    rProperty[XML_w] = rShape->getSize().Width * rConstraint.mfFactor;
                    if (rProperty[XML_w] > rShape->getSize().Width)
                    {
                        rProperty[XML_w] = rShape->getSize().Width;
                    }
                }
                if (rConstraint.mnType == XML_h)
                {
                    rProperty[XML_h] = rShape->getSize().Height * rConstraint.mfFactor;
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
            if (nIncX || nIncY)
                aExtents.read(rDgm, rShape, rConstraints, nAxisType, nParentExtent);

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
                    if (bAny && fReach > 0.0)
                        aExtents.set(pChild->getInternalName(), fReach / nParentExtent);
                }
            }
            // The wishes are handed out as parts of the parent, a spacer of a little less than
            // nothing among them as well: it pulls the next child a little closer, and the whole
            // is what the wishes add up to, Detailed_Process's four nodes and three gaps of 0.035
            // of a node. A layout that walks the place back over a whole child means its
            // children to overlap, and then the wishes are no plain division of the parent.
            const bool bDivideParent = aExtents.isFilled() && !aExtents.hasOverlap();
            std::vector<sal_Int32> aWantedExtent;

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
                        aWantedExtent.push_back(oFactor.value() * nParentExtent);
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
            if (bFillAxis)
            {
                sal_Int32 nWanted = 0;
                for (sal_Int32 nOne : aWantedExtent)
                    nWanted += nOne;
                if (nIncX)
                    aTotalSize.Width = nWanted;
                else
                    aTotalSize.Height = nWanted;
            }

            double fWidthScale = 1.0;
            double fHeightScale = 1.0;
            if (nIncX && aTotalSize.Width > rShape->getSize().Width)
                fWidthScale = static_cast<double>(rShape->getSize().Width) / aTotalSize.Width;
            if (nIncY && aTotalSize.Height > rShape->getSize().Height)
                fHeightScale = static_cast<double>(rShape->getSize().Height) / aTotalSize.Height;
            aSpaceSize.Width *= fWidthScale;
            aSpaceSize.Height *= fHeightScale;

            sal_Int32 nAxisCursor = (nIncX == -1 || nIncY == -1) ? nParentExtent : 0;
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
                if (bFillAxis)
                {
                    if (nIncX)
                        aSize.Width = aWantedExtent[nWantedIndex];
                    else
                        aSize.Height = aWantedExtent[nWantedIndex];
                }
                ++nWantedIndex;
                if (aChildrenToShrink.empty()
                    || aChildrenToShrink.find(aCurrShape->getInternalName())
                           != aChildrenToShrink.end())
                {
                    aSize.Width *= fWidthScale;
                }
                if (aChildrenToShrink.empty()
                    || aChildrenToShrink.find(aCurrShape->getInternalName())
                           != aChildrenToShrink.end())
                {
                    aSize.Height *= fHeightScale;
                }

                // A layout may state every size of a row, along it and across it, as parts of a
                // name of its own, userH say, twice the height. The name is the unit of the whole
                // row then: where the row shrinks the children along it to fit, the unit shrank,
                // and a child's extent across that is a part of the same name shrinks the same
                // way. The unit as it came out is read off the child itself, its extent along
                // over the part of the name that extent was stated as; the extent across is its
                // own part of that, and not the room the child was clamped to.
                {
                    std::optional<double> oAlongOfUnit;
                    std::optional<double> oAcrossOfUnit;
                    sal_Int32 nUnit(0);
                    for (const Constraint& rConstraint : rConstraints)
                    {
                        if (rConstraint.mnFor != XML_ch
                            || rConstraint.msForName != aCurrShape->getInternalName()
                            || !isUserVariable(rConstraint.mnRefType)
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
                if (!aCrossSettledHere.count(aCurrShape->getInternalName()))
                {
                    const auto aChild = aChildConstraints.find(aCurrShape->getInternalName());
                    if (aChild != aChildConstraints.end())
                    {
                        const std::optional<double> oCross(
                            readOwnCrossFactor(aChild->second, nCrossType, nAxisType));
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

                aCurrShape->setSize(aSize);
                aCurrShape->setChildSize(aSize);

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
                }
                if (aCurrPos.X < 0)
                {
                    aCurrPos.X = 0;
                }
                if (aCurrPos.Y < 0)
                {
                    aCurrPos.Y = 0;
                }

                if (bFillAxis)
                {
                    const sal_Int32 nExtent = nIncX ? aSize.Width : aSize.Height;
                    if (nIncX == -1 || nIncY == -1)
                        nAxisCursor -= nExtent;
                    if (nIncX)
                        aCurrPos.X = nAxisCursor;
                    else
                        aCurrPos.Y = nAxisCursor;
                    if (nIncX == 1 || nIncY == 1)
                        nAxisCursor += nExtent;
                }

                aCurrShape->setPosition(aCurrPos);

                aCurrPos.X += nIncX * (aSize.Width + aSpaceSize.Width);
                aCurrPos.Y += nIncY * (aSize.Height + aSpaceSize.Height);

                // connectors should be handled in conn, but we don't have
                // reference to previous and next child, so it's easier here
                if (aCurrShape->getSubType() == XML_conn)
                    aCurrShape->setRotation(nConnectorAngle * PER_DEGREE);
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
            PyraAlg::layoutShapeChildren(rShape);
            break;
        }

        case XML_snake:
        {
            SnakeAlg::layoutShapeChildren(*this, rShape, rConstraints);
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

            TextBodyPtr pTextBody = rShape->getTextBody();
            if (!pTextBody || pTextBody->isEmpty())
                break;

            // adjust text size to fit shape
            if (fFontSize != 0)
            {
                for (auto& aParagraph : pTextBody->getParagraphs())
                    for (auto& aRun : aParagraph->getRuns())
                        if (!aRun->getTextCharacterProperties().moHeight.has_value())
                            aRun->getTextCharacterProperties().moHeight = fFontSize * 100;
            }

            if (!HasCustomText(rDgm, rShape))
            {
                // No customized text properties: enable autofit.
                pTextBody->getTextProperties().maPropertyMap.setProperty(
                    PROP_TextFitToSize, drawing::TextFitToSizeType_AUTOFIT);
            }

            // ECMA-376-1:2016 21.4.7.5 ST_AutoTextRotation (Auto Text Rotation)
            const sal_Int32 nautoTxRot = maMap.count(XML_autoTxRot) ? maMap.find(XML_autoTxRot)->second : XML_upr;
            sal_Int32 nShapeRot = rShape->getRotation();
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
                }
                break;
                case XML_grav:
                {
                    if (nShapeRot > (90 * PER_DEGREE) && nShapeRot < (270 * PER_DEGREE))
                        pTextBody->getTextProperties().moTextPreRotation = -180 * PER_DEGREE;
                }
                break;
                case XML_none:
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
                    if (!aParagraph->getProperties().getParaLeftMargin().has_value())
                    {
                        sal_Int32 nLeftMargin
                            = o3tl::convert(285750 * (nLevel - nStartBulletsAtLevel + 1),
                                            o3tl::Length::emu, o3tl::Length::mm100);
                        aParagraph->getProperties().getParaLeftMargin() = nLeftMargin;
                    }

                    if (!aParagraph->getProperties().getFirstLineIndentation().has_value())
                        aParagraph->getProperties().getFirstLineIndentation()
                            = o3tl::convert(-285750, o3tl::Length::emu, o3tl::Length::mm100);

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
            // Take the nth color from the color list in case we are the nth shape in a
            // <dgm:forEach> loop.
            const DiagramColor& rColor=aColor->second;
            if( !rColor.maFillColors.empty() )
                rShape->getShapeStyleRefs()[XML_fillRef].maPhClr = DiagramColor::getColorByIndex(rColor.maFillColors, nCurrIdx);
            if( !rColor.maLineColors.empty() )
            {
                const Color aLineColor(
                    DiagramColor::getColorByIndex(rColor.maLineColors, nCurrIdx));
                rShape->getShapeStyleRefs()[XML_lnRef].maPhClr = aLineColor;

                // A line style of a theme carries the width and the dash of a line and leaves the
                // colour to whoever points at it. A shape that defines no line takes the colour
                // that the Diagram gives it as a line, while the width and the dash are from the style.
                LineProperties& rLineProperties(rShape->getLineProperties());

                if (!rLineProperties.maLineFill.moFillType.has_value())
                {
                    rLineProperties.maLineFill.moFillType = XML_solidFill;
                    rLineProperties.maLineFill.maFillColor = aLineColor;
                }
            }
            if( !rColor.maEffectColors.empty() )
                rShape->getShapeStyleRefs()[XML_effectRef].maPhClr = DiagramColor::getColorByIndex(rColor.maEffectColors, nCurrIdx);
            if( !rColor.maTextFillColors.empty() )
                rShape->getShapeStyleRefs()[XML_fontRef].maPhClr = DiagramColor::getColorByIndex(rColor.maTextFillColors, nCurrIdx);
        }
    }

    // even if no data node found, successful anyway. it's
    // contained at the layoutnode
    return true;
}

void ShapeAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
