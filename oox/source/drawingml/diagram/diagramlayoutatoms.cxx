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

#include <set>

#include "layoutatomvisitorbase.hxx"

#include <basegfx/numeric/ftools.hxx>
#include <sal/log.hxx>

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
oox::OptValue<sal_Int32> findProperty(const oox::drawingml::LayoutPropertyMap& rProperties,
                                      const OUString& rInternalName, sal_Int32 nProperty)
{
    oox::OptValue<sal_Int32> oRet;

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
void SnakeAlg::layoutShapeChildren(const AlgAtom::ParamMap& rMap, const ShapePtr& rShape,
                                   const std::vector<Constraint>& rConstraints)
{
    if (rShape->getChildren().empty() || rShape->getSize().Width == 0
        || rShape->getSize().Height == 0)
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
    LayoutPropertyMap aPropertiesByName;
    std::map<sal_Int32, LayoutProperty> aPropertiesByType;
    LayoutProperty& rParent = aPropertiesByName[""];
    rParent[XML_w] = fShapeWidth;
    rParent[XML_h] = fShapeHeight;
    for (const auto& rConstr : rConstraints)
    {
        if (rConstr.mnRefType == XML_w || rConstr.mnRefType == XML_h)
        {
            if (rConstr.mnType == XML_sp && rConstr.msForName.isEmpty())
                fSpaceFromConstraint = rConstr.mfFactor;
        }

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

        if (rConstr.mnPointType == XML_none)
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
    // Defaults in case not provided by constraints.
    double fSpace = bSpaceFromConstraints ? fSpaceFromConstraint : 0.3;
    double fAspectRatio = 0.54; // diagram should not spill outside, earlier it was 0.6

    sal_Int32 nCol = 1;
    sal_Int32 nRow = 1;
    sal_Int32 nMaxRowWidth = 0;
    if (nCount <= fChildAspectRatio)
        // Child aspect ratio request (width/height) is N, and we have at most N shapes.
        // This means we don't need multiple columns.
        nRow = nCount;
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

    SAL_INFO("oox.drawingml", "Snake layout grid: " << nCol << "x" << nRow);

    sal_Int32 nWidth = rShape->getSize().Width / (nCol + (nCol - 1) * fSpace);
    awt::Size aChildSize(nWidth, nWidth * fAspectRatio);
    if (nCol == 1 && nRow > 1)
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
                        // position first child of last row
                        if (bWidthsFromConstraints)
                        {
                            aCurrPos.X = nStartX;
                        }
                        else
                        {
                            // Can assume that all child shape has the same width.
                            aCurrPos.X
                                = nStartX
                                  + (nIncX * (aCurrSize.Width + fSpace * aCurrSize.Width)) / 2;
                        }
                    }
                    else
                        // if not last row, positions first child of that row
                        aCurrPos.X = nStartX;
                    aCurrPos.Y += nIncY * (nRowHeight + fSpace * nRowHeight);
                    nColIdx = 0;
                    nRowHeight = 0;
                }

                // positions children in the last row.
                if (index % nCol != 0 && index >= 3 && ((index / nCol) + 1) == nRow)
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
}

IteratorAttr::IteratorAttr( )
    : mnCnt( -1 )
    , mbHideLastTrans( true )
    , mnPtType( 0 )
    , mnSt( 0 )
    , mnStep( 1 )
{
}

void IteratorAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    AttributeList attr( xAttr );
    maAxis = attr.getTokenList(XML_axis);
    mnCnt = attr.getInteger( XML_cnt, -1 );
    mbHideLastTrans = attr.getBool( XML_hideLastTrans, true );
    mnSt = attr.getInteger( XML_st, 0 );
    mnStep = attr.getInteger( XML_step, 1 );

    // better to keep first token instead of error when multiple values
    std::vector<sal_Int32> aPtTypes = attr.getTokenList(XML_ptType);
    mnPtType = aPtTypes.empty() ? XML_all : aPtTypes.front();
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

LayoutAtomPtr ForEachAtom::getRefAtom()
{
    if (!msRef.isEmpty())
    {
        const LayoutAtomMap& rLayoutAtomMap = getLayoutNode().getDiagram().getLayout()->getLayoutAtomMap();
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
OUString navigate(LayoutNode& rLayoutNode, sal_Int32 nType, std::u16string_view rFrom,
                  bool bSourceToDestination)
{
    for (const auto& rConnection : rLayoutNode.getDiagram().getData()->getConnections())
    {
        if (rConnection.mnType != nType)
            continue;

        if (bSourceToDestination)
        {
            if (rConnection.msSourceId == rFrom)
                return rConnection.msDestId;
        }
        else
        {
            if (rConnection.msDestId == rFrom)
                return rConnection.msSourceId;
        }
    }

    return OUString();
}

sal_Int32 calcMaxDepth(std::u16string_view rNodeName, const dgm::Connections& rConnections)
{
    sal_Int32 nMaxLength = 0;
    for (auto const& aCxn : rConnections)
        if (aCxn.mnType == XML_parOf && aCxn.msSourceId == rNodeName)
            nMaxLength = std::max(nMaxLength, calcMaxDepth(aCxn.msDestId, rConnections) + 1);

    return nMaxLength;
}
}

sal_Int32 ConditionAtom::getNodeCount(const dgm::Point* pPresPoint) const
{
    sal_Int32 nCount = 0;
    OUString sNodeId = pPresPoint->msPresentationAssociationId;

    // HACK: special case - count children of first child
    if (maIter.maAxis.size() == 2 && maIter.maAxis[0] == XML_ch && maIter.maAxis[1] == XML_ch)
        sNodeId = navigate(mrLayoutNode, XML_parOf, sNodeId, /*bSourceToDestination*/ true);

    if (!sNodeId.isEmpty())
    {
        for (const auto& aCxn : mrLayoutNode.getDiagram().getData()->getConnections())
            if (aCxn.mnType == XML_parOf && aCxn.msSourceId == sNodeId)
                nCount++;
    }

    return nCount;
}

bool ConditionAtom::getDecision(const dgm::Point* pPresPoint) const
{
    if (mIsElse)
        return true;
    if (!pPresPoint)
        return false;

    switch (maCond.mnFunc)
    {
    case XML_var:
    {
        if (maCond.mnArg == XML_dir)
            return compareResult(maCond.mnOp, pPresPoint->mnDirection, maCond.mnVal);
        else if (maCond.mnArg == XML_hierBranch)
        {
            sal_Int32 nHierarchyBranch = pPresPoint->moHierarchyBranch.get(XML_std);
            if (!pPresPoint->moHierarchyBranch.has())
            {
                // If <dgm:hierBranch> is missing in the current presentation
                // point, ask the parent.
                OUString aParent = navigate(mrLayoutNode, XML_presParOf, pPresPoint->msModelId,
                                            /*bSourceToDestination*/ false);
                DiagramData::PointNameMap& rPointNameMap
                    = mrLayoutNode.getDiagram().getData()->getPointNameMap();
                auto it = rPointNameMap.find(aParent);
                if (it != rPointNameMap.end())
                {
                    const dgm::Point* pParent = it->second;
                    if (pParent->moHierarchyBranch.has())
                        nHierarchyBranch = pParent->moHierarchyBranch.get();
                }
            }
            return compareResult(maCond.mnOp, nHierarchyBranch, maCond.mnVal);
        }
        break;
    }

    case XML_cnt:
        return compareResult(maCond.mnOp, getNodeCount(pPresPoint), maCond.msVal.toInt32());

    case XML_maxDepth:
    {
        sal_Int32 nMaxDepth = calcMaxDepth(pPresPoint->msPresentationAssociationId, mrLayoutNode.getDiagram().getData()->getConnections());
        return compareResult(maCond.mnOp, nMaxDepth, maCond.msVal.toInt32());
    }

    case XML_depth:
    case XML_pos:
    case XML_revPos:
    case XML_posEven:
    case XML_posOdd:
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
            case XML_lMarg:
            case XML_rMarg:
            case XML_tMarg:
            case XML_bMarg:
                bRequireForName = false;
                break;
        }
        switch (maConstraint.mnPointType)
        {
            case XML_sibTrans:
                bRequireForName = false;
                break;
        }
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
    if (rShape->getChildren().empty())
        return (rShape->getSubType() != XML_conn) ? 1 : 0;

    sal_Int32 nDir = XML_fromL;
    if (mnType == XML_hierRoot)
        nDir = XML_fromT;
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
/**
 * Apply rConstraint to the rProperties shared layout state.
 *
 * Note that the order in which constraints are applied matters, given that constraints can refer to
 * each other, and in case A depends on B and A is applied before B, the effect of A won't be
 * updated when B is applied.
 */
void ApplyConstraintToLayout(const Constraint& rConstraint, LayoutPropertyMap& rProperties)
{
    // TODO handle the case when we have ptType="...", not forName="...".
    if (rConstraint.msForName.isEmpty())
    {
        return;
    }

    const LayoutPropertyMap::const_iterator aRef = rProperties.find(rConstraint.msRefForName);
    if (aRef == rProperties.end())
        return;

    const LayoutProperty::const_iterator aRefType = aRef->second.find(rConstraint.mnRefType);
    if (aRefType != aRef->second.end())
        rProperties[rConstraint.msForName][rConstraint.mnType]
            = aRefType->second * rConstraint.mfFactor;
    else
    {
        // Values are never in EMU, while oox::drawingml::Shape position and size are always in
        // EMU.
        double fUnitFactor = 0;
        if (isFontUnit(rConstraint.mnRefType))
            // Points -> EMU.
            fUnitFactor = EMU_PER_PT;
        else
            // Millimeters -> EMU.
            fUnitFactor = EMU_PER_HMM * 100;
        rProperties[rConstraint.msForName][rConstraint.mnType]
            = rConstraint.mfValue * fUnitFactor;
    }
}

/// Does the first data node of this shape have customized text properties?
bool HasCustomText(const ShapePtr& rShape, LayoutNode& rLayoutNode)
{
    const PresPointShapeMap& rPresPointShapeMap
        = rLayoutNode.getDiagram().getLayout()->getPresPointShapeMap();
    const DiagramData::StringMap& rPresOfNameMap
        = rLayoutNode.getDiagram().getData()->getPresOfNameMap();
    const DiagramData::PointNameMap& rPointNameMap
        = rLayoutNode.getDiagram().getData()->getPointNameMap();
    // Get the first presentation node of the shape.
    const dgm::Point* pPresNode = nullptr;
    for (const auto& rPair : rPresPointShapeMap)
    {
        if (rPair.second == rShape)
        {
            pPresNode = rPair.first;
            break;
        }
    }
    // Get the first data node of the presentation node.
    dgm::Point* pDataNode = nullptr;
    if (pPresNode)
    {
        auto itPresToData = rPresOfNameMap.find(pPresNode->msModelId);
        if (itPresToData != rPresOfNameMap.end())
        {
            for (const auto& rPair : itPresToData->second)
            {
                const DiagramData::SourceIdAndDepth& rItem = rPair.second;
                auto it = rPointNameMap.find(rItem.msSourceId);
                if (it != rPointNameMap.end())
                {
                    pDataNode = it->second;
                    break;
                }
            }
        }
    }

    // If we have a data node, see if its text is customized or not.
    if (pDataNode)
    {
        return pDataNode->mbCustomText;
    }

    return false;
}
}

void AlgAtom::layoutShape(const ShapePtr& rShape, const std::vector<Constraint>& rConstraints,
                          const std::vector<Rule>& rRules)
{
    if (mnType != XML_lin)
    {
        // TODO Handle spacing from constraints for non-lin algorithms as well.
        rShape->getChildren().erase(
            std::remove_if(rShape->getChildren().begin(), rShape->getChildren().end(),
                           [](const ShapePtr& aChild) {
                               return aChild->getServiceName() == "com.sun.star.drawing.GroupShape"
                                      && aChild->getChildren().empty();
                           }),
            rShape->getChildren().end());
    }

    switch(mnType)
    {
        case XML_composite:
        {
            // layout shapes using basic constraints

            LayoutPropertyMap aProperties;
            LayoutProperty& rParent = aProperties[""];

            sal_Int32 nParentXOffset = 0;

            // Track min/max vertical positions, so we can center everything at the end, if needed.
            sal_Int32 nVertMin = std::numeric_limits<sal_Int32>::max();
            sal_Int32 nVertMax = 0;

            if (mfAspectRatio != 1.0)
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

            for (const auto & rConstr : rConstraints)
            {
                // Apply direct constraints for all layout nodes.
                ApplyConstraintToLayout(rConstr, aProperties);
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

                    ApplyConstraintToLayout(rConstr, aProperties);
                }

                // Apply constraints from the child layout node for this child shape.
                // This builds on top of the own parent state + the state of previous shapes in the
                // same composite algorithm.
                const LayoutNode& rLayoutNode = getLayoutNode();
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

                        // Either an absolute value or a factor of a property.
                        if (rConstraint.mfValue == 0.0 && rConstraint.mnRefType == XML_none)
                        {
                            continue;
                        }

                        Constraint aConstraint(rConstraint);
                        aConstraint.msForName = pLayoutNode->getName();
                        aConstraint.msRefForName = pLayoutNode->getName();

                        ApplyConstraintToLayout(aConstraint, aProperties);
                    }
                }

                awt::Size aSize = rShape->getSize();
                awt::Point aPos(0, 0);

                const LayoutPropertyMap::const_iterator aPropIt = aProperties.find(aCurrShape->getInternalName());
                if (aPropIt != aProperties.end())
                {
                    const LayoutProperty& rProp = aPropIt->second;
                    LayoutProperty::const_iterator it, it2;

                    if ( (it = rProp.find(XML_w)) != rProp.end() )
                        aSize.Width = std::min(it->second, rShape->getSize().Width);
                    if ( (it = rProp.find(XML_h)) != rProp.end() )
                        aSize.Height = std::min(it->second, rShape->getSize().Height);

                    if ( (it = rProp.find(XML_l)) != rProp.end() )
                        aPos.X = it->second;
                    else if ( (it = rProp.find(XML_ctrX)) != rProp.end() )
                        aPos.X = it->second - aSize.Width/2;
                    else if ((it = rProp.find(XML_r)) != rProp.end())
                        aPos.X = it->second - aSize.Width;

                    if ( (it = rProp.find(XML_t)) != rProp.end())
                        aPos.Y = it->second;
                    else if ( (it = rProp.find(XML_ctrY)) != rProp.end() )
                        aPos.Y = it->second - aSize.Height/2;
                    else if ((it = rProp.find(XML_b)) != rProp.end())
                        aPos.Y = it->second - aSize.Height;

                    if ( (it = rProp.find(XML_l)) != rProp.end() && (it2 = rProp.find(XML_r)) != rProp.end() )
                        aSize.Width = it2->second - it->second;
                    if ( (it = rProp.find(XML_t)) != rProp.end() && (it2 = rProp.find(XML_b)) != rProp.end() )
                        aSize.Height = it2->second - it->second;

                    aPos.X += nParentXOffset;
                    aSize.Width = std::min(aSize.Width, rShape->getSize().Width - aPos.X);
                    aSize.Height = std::min(aSize.Height, rShape->getSize().Height - aPos.Y);
                }
                else
                    SAL_WARN("oox.drawingml", "composite layout properties not found for shape " << aCurrShape->getInternalName());

                aCurrShape->setSize(aSize);
                aCurrShape->setChildSize(aSize);
                aCurrShape->setPosition(aPos);

                nVertMin = std::min(aPos.Y, nVertMin);
                nVertMax = std::max(aPos.Y + aSize.Height, nVertMax);

                NamedShapePairs& rDiagramFontHeights
                    = getLayoutNode().getDiagram().getShape()->getDiagramFontHeights();
                auto it = rDiagramFontHeights.find(aCurrShape->getInternalName());
                if (it != rDiagramFontHeights.end())
                {
                    // Internal name matches: put drawingml::Shape to the relevant group, for
                    // synchronized font height handling.
                    it->second.insert({ aCurrShape, {} });
                }
            }

            // See if all vertical space is used or we have to center the content.
            if (nVertMin >= 0 && nVertMin <= nVertMax && nVertMax <= rParent[XML_h])
            {
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

            // Parse constraints to adjust the size.
            std::vector<Constraint> aDirectConstraints;
            const LayoutNode& rLayoutNode = getLayoutNode();
            for (const auto& pChild : rLayoutNode.getChildren())
            {
                auto pConstraintAtom = dynamic_cast<ConstraintAtom*>(pChild.get());
                if (pConstraintAtom)
                    pConstraintAtom->parseConstraint(aDirectConstraints, /*bRequireForName=*/false);
            }

            LayoutPropertyMap aProperties;
            LayoutProperty& rParent = aProperties[""];
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
            const awt::Size aChildSize(rShape->getSize().Width / 4, rShape->getSize().Height / 4);
            const awt::Size aConnectorSize(rShape->getSize().Width / 12, rShape->getSize().Height / 12);
            const sal_Int32 nRadius = std::min(
                (rShape->getSize().Width - aChildSize.Width) / 2,
                (rShape->getSize().Height - aChildSize.Height) / 2);

            std::vector<oox::drawingml::ShapePtr> aCycleChildren = rShape->getChildren();

            if (nctrShpMap == XML_fNode)
            {
                // first node placed in center, others around
                oox::drawingml::ShapePtr pCenterShape = aCycleChildren.front();
                aCycleChildren.erase(aCycleChildren.begin());
                const awt::Point aCurrPos(aCenter.Width - aChildSize.Width / 2,
                                          aCenter.Height - aChildSize.Height / 2);
                pCenterShape->setPosition(aCurrPos);
                pCenterShape->setSize(aChildSize);
                pCenterShape->setChildSize(aChildSize);
            }

            const sal_Int32 nShapes = aCycleChildren.size();
            if (nShapes)
            {
                const sal_Int32 nConnectorRadius = nRadius * cos(basegfx::deg2rad(nSpanAngle / nShapes));
                const sal_Int32 nConnectorAngle = nSpanAngle > 0 ? 0 : 180;

                sal_Int32 idx = 0;
                for (auto & aCurrShape : aCycleChildren)
                {
                    const double fAngle = static_cast<double>(idx)*nSpanAngle/nShapes + nStartAngle;
                    awt::Size aCurrSize = aChildSize;
                    sal_Int32 nCurrRadius = nRadius;
                    if (aCurrShape->getSubType() == XML_conn)
                    {
                        aCurrSize = aConnectorSize;
                        nCurrRadius = nConnectorRadius;
                    }
                    const awt::Point aCurrPos(
                        aCenter.Width + nCurrRadius*sin(basegfx::deg2rad(fAngle)) - aCurrSize.Width/2,
                        aCenter.Height - nCurrRadius*cos(basegfx::deg2rad(fAngle)) - aCurrSize.Height/2);

                    aCurrShape->setPosition(aCurrPos);
                    aCurrShape->setSize(aCurrSize);
                    aCurrShape->setChildSize(aCurrSize);

                    if (nRotationPath == XML_alongPath)
                        aCurrShape->setRotation(fAngle * PER_DEGREE);

                    // connectors should be handled in conn, but we don't have
                    // reference to previous and next child, so it's easier here
                    if (aCurrShape->getSubType() == XML_conn)
                        aCurrShape->setRotation((nConnectorAngle + fAngle) * PER_DEGREE);

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

            // hierRoot is the manager -> employees vertical linear path,
            // hierChild is the first employee -> last employee horizontal
            // linear path.
            sal_Int32 nDir = XML_fromL;
            if (mnType == XML_hierRoot)
                nDir = XML_fromT;
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

            const double fSpaceWidth = 0.1;
            const double fSpaceHeight = 0.3;

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

            awt::Size aConnectorSize = aChildSize;
            aConnectorSize.Width = 1;

            awt::Point aChildPos(0, 0);

            // indent children to show they are descendants, not siblings
            if (mnType == XML_hierChild && nHorizontalShapesCount == 1)
            {
                const double fChildIndent = 0.1;
                aChildPos.X = aChildSize.Width * fChildIndent;
                aChildSize.Width *= (1 - 2 * fChildIndent);
            }

            sal_Int32 nIdx = 0;
            sal_Int32 nRowHeight = 0;
            for (auto& pChild : rShape->getChildren())
            {
                pChild->setPosition(aChildPos);

                if (mnType == XML_hierChild && pChild->getSubType() == XML_conn)
                {
                    // Connectors should not influence the position of
                    // non-connect shapes.
                    pChild->setSize(aConnectorSize);
                    pChild->setChildSize(aConnectorSize);
                    continue;
                }

                awt::Size aCurrSize = aChildSize;
                aCurrSize.Height *= pChild->getVerticalShapesCount() + (pChild->getVerticalShapesCount() - 1) * fSpaceHeight;

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
            case XML_fromT: nConnectorAngle = 270; break;
            case XML_fromB: nConnectorAngle = 90; break;
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
                    NamedShapePairs& rDiagramFontHeights
                        = getLayoutNode().getDiagram().getShape()->getDiagramFontHeights();
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
                                    oox::OptValue<sal_Int32> oWidth = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);
                                    bScaleDownEmptySpacing = oWidth.has() && oWidth.get() > 0;
                                }
                                if (!bScaleDownEmptySpacing && (nDir == XML_fromT || nDir == XML_fromB))
                                {
                                    oox::OptValue<sal_Int32> oHeight = findProperty(aProperties, aCurrShape->getInternalName(), XML_h);
                                    bScaleDownEmptySpacing = oHeight.has() && oHeight.get() > 0;
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
            else
            {
                // TODO Handle spacing from constraints without rules as well.
                rShape->getChildren().erase(
                    std::remove_if(rShape->getChildren().begin(), rShape->getChildren().end(),
                                   [](const ShapePtr& aChild) {
                                       return aChild->getServiceName()
                                                  == "com.sun.star.drawing.GroupShape"
                                              && aChild->getChildren().empty();
                                   }),
                    rShape->getChildren().end());
                fCount = rShape->getChildren().size();
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
                oox::OptValue<sal_Int32> oWidth = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);
                oox::OptValue<sal_Int32> oHeight = findProperty(aProperties, aCurrShape->getInternalName(), XML_h);
                awt::Size aSize = aChildSize;
                if (oWidth.has())
                    aSize.Width = oWidth.get();
                if (oHeight.has())
                    aSize.Height = oHeight.get();
                aTotalSize.Width += aSize.Width;
                aTotalSize.Height += aSize.Height;
            }

            aTotalSize.Width += (fCount-1) * aSpaceSize.Width;
            aTotalSize.Height += (fCount-1) * aSpaceSize.Height;

            double fWidthScale = 1.0;
            double fHeightScale = 1.0;
            if (nIncX && aTotalSize.Width > rShape->getSize().Width)
                fWidthScale = static_cast<double>(rShape->getSize().Width) / aTotalSize.Width;
            if (nIncY && aTotalSize.Height > rShape->getSize().Height)
                fHeightScale = static_cast<double>(rShape->getSize().Height) / aTotalSize.Height;
            aSpaceSize.Width *= fWidthScale;
            aSpaceSize.Height *= fHeightScale;

            for (auto& aCurrShape : rShape->getChildren())
            {
                // Extract properties relevant for this shape from constraints.
                oox::OptValue<sal_Int32> oWidth = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);
                oox::OptValue<sal_Int32> oHeight = findProperty(aProperties, aCurrShape->getInternalName(), XML_h);

                awt::Size aSize = aChildSize;
                if (oWidth.has())
                    aSize.Width = oWidth.get();
                if (oHeight.has())
                    aSize.Height = oHeight.get();
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
                aCurrShape->setSize(aSize);
                aCurrShape->setChildSize(aSize);

                // center in the other axis - probably some parameter controls it
                if (nIncX)
                    aCurrPos.Y = (rShape->getSize().Height - aSize.Height) / 2;
                if (nIncY)
                    aCurrPos.X = (rShape->getSize().Width - aSize.Width) / 2;
                if (aCurrPos.X < 0)
                {
                    aCurrPos.X = 0;
                }
                if (aCurrPos.Y < 0)
                {
                    aCurrPos.Y = 0;
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
            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

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
            aCurrPos.X = fAspectRatio*aChildSize.Width*(nCount-1);
            aCurrPos.Y = fAspectRatio*aChildSize.Height;

            for (auto & aCurrShape : rShape->getChildren())
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
            break;
        }

        case XML_snake:
        {
            SnakeAlg::layoutShapeChildren(maMap, rShape, rConstraints);
            break;
        }

        case XML_sp:
        {
            // HACK: Handled one level higher. Or rather, planned to
            // HACK: text should appear only in tx node; we're assigning it earlier, so let's remove it here
            rShape->setTextBody(TextBodyPtr());
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
                    sal_Int32 nValue = rShape->getSize().Width * fFactor / EMU_PER_HMM;

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
                        if (!aRun->getTextCharacterProperties().moHeight.has())
                            aRun->getTextCharacterProperties().moHeight = fFontSize * 100;
            }

            if (!HasCustomText(rShape, getLayoutNode()))
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
                    pTextBody->getTextProperties().moRotation = n90x * 90 * PER_DEGREE;
                }
                break;
                case XML_grav:
                {
                    if (nShapeRot > (90 * PER_DEGREE) && nShapeRot < (270 * PER_DEGREE))
                        pTextBody->getTextProperties().moRotation = -180 * PER_DEGREE;
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
                        sal_Int32 nLeftMargin = 285750 * (nLevel - nStartBulletsAtLevel + 1) / EMU_PER_HMM;
                        aParagraph->getProperties().getParaLeftMargin() = nLeftMargin;
                    }

                    if (!aParagraph->getProperties().getFirstLineIndentation().has_value())
                        aParagraph->getProperties().getFirstLineIndentation() = -285750 / EMU_PER_HMM;

                    // It is not possible to change the bullet style for text.
                    aParagraph->getProperties().getBulletList().setBulletChar(u"•");
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

void LayoutNode::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

bool LayoutNode::setupShape( const ShapePtr& rShape, const dgm::Point* pPresNode, sal_Int32 nCurrIdx ) const
{
    SAL_INFO(
        "oox.drawingml",
        "Filling content from layout node named \"" << msName
            << "\", modelId \"" << pPresNode->msModelId << "\"");

    // have the presentation node - now, need the actual data node:
    const DiagramData::StringMap::const_iterator aNodeName = mrDgm.getData()->getPresOfNameMap().find(
        pPresNode->msModelId);
    if( aNodeName != mrDgm.getData()->getPresOfNameMap().end() )
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
            const DiagramData::SourceIdAndDepth& rItem = rPair.second;
            DiagramData::PointNameMap& rMap = mrDgm.getData()->getPointNameMap();
            // pPresNode is the presentation node of the aDataNode2 data node.
            DiagramData::PointNameMap::const_iterator aDataNode2 = rMap.find(rItem.msSourceId);
            if (aDataNode2 == rMap.end())
            {
                //busted, skip it
                continue;
            }

            if (!aDataNode2->second->mpShape)
            {
                //busted, skip it
                continue;
            }

            rShape->setDataNodeType(aDataNode2->second->mnType);

            if (rItem.mnDepth == 0)
            {
                // grab shape attr from topmost element(s)
                rShape->getShapeProperties() = aDataNode2->second->mpShape->getShapeProperties();
                rShape->getLineProperties() = aDataNode2->second->mpShape->getLineProperties();
                rShape->getFillProperties() = aDataNode2->second->mpShape->getFillProperties();
                rShape->getCustomShapeProperties() = aDataNode2->second->mpShape->getCustomShapeProperties();
                rShape->setMasterTextListStyle( aDataNode2->second->mpShape->getMasterTextListStyle() );

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
                rShape->getLineProperties() = aDataNode2->second->mpShape->getLineProperties();
                rShape->getFillProperties() = aDataNode2->second->mpShape->getFillProperties();
            }

            // append text with right outline level
            if( aDataNode2->second->mpShape->getTextBody() &&
                !aDataNode2->second->mpShape->getTextBody()->getParagraphs().empty() &&
                !aDataNode2->second->mpShape->getTextBody()->getParagraphs().front()->getRuns().empty() )
            {
                TextBodyPtr pTextBody=rShape->getTextBody();
                if( !pTextBody )
                {
                    pTextBody = std::make_shared<TextBody>();

                    // also copy text attrs
                    pTextBody->getTextListStyle() =
                        aDataNode2->second->mpShape->getTextBody()->getTextListStyle();
                    pTextBody->getTextProperties() =
                        aDataNode2->second->mpShape->getTextBody()->getTextProperties();

                    rShape->setTextBody(pTextBody);
                }

                const TextParagraphVector& rSourceParagraphs
                    = aDataNode2->second->mpShape->getTextBody()->getParagraphs();
                for (const auto& pSourceParagraph : rSourceParagraphs)
                {
                    TextParagraph& rPara = pTextBody->addParagraph();
                    if (rItem.mnDepth != -1)
                        rPara.getProperties().setLevel(rItem.mnDepth);

                    for (const auto& pRun : pSourceParagraph->getRuns())
                        rPara.addRun(pRun);
                    const TextBodyPtr& rBody = aDataNode2->second->mpShape->getTextBody();
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
        if (pPresNode->mpShape)
            rShape->getFillProperties().assignUsed(pPresNode->mpShape->getFillProperties());
    }

    // TODO(Q1): apply styling & coloring - take presentation
    // point's presStyleLbl for both style & color
    // if not found use layout node's styleLbl
    // however, docs are a bit unclear on this
    OUString aStyleLabel = pPresNode->msPresentationLayoutStyleLabel;
    if (aStyleLabel.isEmpty())
        aStyleLabel = msStyleLabel;
    if( !aStyleLabel.isEmpty() )
    {
        const DiagramQStyleMap::const_iterator aStyle = mrDgm.getStyles().find(aStyleLabel);
        if( aStyle != mrDgm.getStyles().end() )
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

        const DiagramColorMap::const_iterator aColor = mrDgm.getColors().find(aStyleLabel);
        if( aColor != mrDgm.getColors().end() )
        {
            // Take the nth color from the color list in case we are the nth shape in a
            // <dgm:forEach> loop.
            const DiagramColor& rColor=aColor->second;
            if( !rColor.maFillColors.empty() )
                rShape->getShapeStyleRefs()[XML_fillRef].maPhClr = DiagramColor::getColorByIndex(rColor.maFillColors, nCurrIdx);
            if( !rColor.maLineColors.empty() )
                rShape->getShapeStyleRefs()[XML_lnRef].maPhClr = DiagramColor::getColorByIndex(rColor.maLineColors, nCurrIdx);
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
