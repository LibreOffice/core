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
#include <tools/gen.hxx>

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

/// Determines the connector shape type from a linear alg.
sal_Int32 getConnectorType(const oox::drawingml::LayoutNode* pNode)
{
    sal_Int32 nType = oox::XML_rightArrow;

    if (!pNode)
        return nType;

    // This is cheaper than visiting the whole sub-tree.
    if (pNode->getName().startsWith("hierChild"))
        return oox::XML_bentConnector3;

    for (const auto& pChild : pNode->getChildren())
    {
        auto pAlgAtom = dynamic_cast<oox::drawingml::AlgAtom*>(pChild.get());
        if (!pAlgAtom)
            continue;

        switch (pAlgAtom->getType())
        {
            case oox::XML_lin:
            {
                sal_Int32 nDir = oox::XML_fromL;
                if (pAlgAtom->getMap().count(oox::XML_linDir))
                    nDir = pAlgAtom->getMap().find(oox::XML_linDir)->second;

                switch (nDir)
                {
                    case oox::XML_fromL:
                        nType = oox::XML_rightArrow;
                        break;
                    case oox::XML_fromR:
                        nType = oox::XML_leftArrow;
                        break;
                }
                break;
            }
            case oox::XML_hierChild:
            {
                // TODO <dgm:param type="connRout" val="..."/> should be able
                // to customize this.
                nType = oox::XML_bentConnector3;
                break;
            }
        }
    }

    return nType;
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

/**
 * Calculates the offset and scaling for pShape (laid out with the hierChild
 * algorithm) based on the siblings of pParent.
 */
void calculateHierChildOffsetScale(const oox::drawingml::ShapePtr& pShape,
                                   const oox::drawingml::LayoutNode* pParent, sal_Int32& rXOffset,
                                   double& rWidthScale)
{
    if (!pParent)
        return;

    const std::vector<oox::drawingml::ShapePtr>& rParents = pParent->getNodeShapes();
    for (size_t nParent = 0; nParent < rParents.size(); ++nParent)
    {
        const oox::drawingml::ShapePtr& pParentShape = rParents[nParent];
        const std::vector<oox::drawingml::ShapePtr>& rChildren = pParentShape->getChildren();
        if (std::none_of(rChildren.begin(), rChildren.end(),
                         [pShape](const oox::drawingml::ShapePtr& pChild) { return pChild == pShape; }))
            // This is not our parent.
            continue;

        if (nParent > 0)
        {
            if (rParents[nParent - 1]->getChildren().size() == 1)
            {
                // Previous sibling of our parent has no children: can use that
                // space, so shift to the left and scale up.
                rWidthScale += 1.0;
                rXOffset -= pShape->getSize().Width;
            }
        }
        if (nParent < rParents.size() - 1)
        {
            if (rParents[nParent + 1]->getChildren().size() == 1)
                // Next sibling of our parent has no children: can use that
                // space, so scale up.
                rWidthScale += 1.0;
        }
    }
}

/// Sets the position and size of a connector inside a hierChild algorithm.
void setHierChildConnPosSize(const oox::drawingml::ShapePtr& pShape)
{
    // Connect to the top center of the child.
    awt::Point aShapePoint = pShape->getPosition();
    awt::Size aShapeSize = pShape->getSize();
    tools::Rectangle aRectangle(Point(aShapePoint.X, aShapePoint.Y),
                                Size(aShapeSize.Width, aShapeSize.Height));
    Point aTo = aRectangle.TopCenter();

    // Connect from the bottom center of the parent.
    Point aFrom = aTo;
    aFrom.setY(aFrom.getY() - aRectangle.getHeight() * 0.3);

    tools::Rectangle aRect(aFrom, aTo);
    aRect.Justify();
    aShapePoint = awt::Point(aRect.Left(), aRect.Top());
    aShapeSize = awt::Size(aRect.getWidth(), aRect.getHeight());
    pShape->setPosition(aShapePoint);
    pShape->setSize(aShapeSize);
}
}

namespace oox { namespace drawingml {

IteratorAttr::IteratorAttr( )
    : mnAxis( 0 )
    , mnCnt( -1 )
    , mbHideLastTrans( false )
    , mnPtType( 0 )
    , mnSt( 0 )
    , mnStep( 1 )
{
}

void IteratorAttr::loadFromXAttr( const Reference< XFastAttributeList >& xAttr )
{
    AttributeList attr( xAttr );
    mnAxis = xAttr->getOptionalValueToken( XML_axis, 0 );
    mnCnt = attr.getInteger( XML_cnt, -1 );
    mbHideLastTrans = attr.getBool( XML_hideLastTrans, false );
    mnPtType = xAttr->getOptionalValueToken( XML_ptType, 0 );
    mnSt = attr.getInteger( XML_st, 0 );
    mnStep = attr.getInteger( XML_step, 1 );
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

void ChooseAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

const std::vector<LayoutAtomPtr>& ChooseAtom::getChildren() const
{
    for (const auto& pChild : mpChildNodes)
    {
        const ConditionAtomPtr pCond = std::dynamic_pointer_cast<ConditionAtom>(pChild);
        if (pCond && pCond->getDecision())
            return pCond->getChildren();
    }
    return maEmptyChildren;
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

const dgm::Point* ConditionAtom::getPresNode() const
{
    const DiagramData::PointsNameMap& rPoints = mrLayoutNode.getDiagram().getData()->getPointsPresNameMap();
    DiagramData::PointsNameMap::const_iterator aDataNode = rPoints.find(mrLayoutNode.getName());
    if (aDataNode != rPoints.end())
    {
        SAL_WARN_IF(aDataNode->second.size() > 1, "oox.drawingml", "multiple nodes found; taking first one");
        return aDataNode->second.front();
    }
    return nullptr;
}

namespace
{
/**
 * Takes the connection list from rLayoutNode, navigates from rFrom on an edge
 * of type nType, using a direction determined by bSourceToDestination.
 */
OUString navigate(const LayoutNode& rLayoutNode, sal_Int32 nType, const OUString& rFrom,
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
}

sal_Int32 ConditionAtom::getNodeCount() const
{
    sal_Int32 nCount = 0;
    const dgm::Point* pPoint = getPresNode();
    if (pPoint)
    {
        OUString sNodeId = "";

        sNodeId
            = navigate(mrLayoutNode, XML_presOf, pPoint->msModelId, /*bSourceToDestination*/ false);

        if (sNodeId.isEmpty())
        {
            // The current layout node is not a presentation of anything. Look
            // up the first presentation child of the layout node.
            OUString sFirstPresChildId = navigate(mrLayoutNode, XML_presParOf, pPoint->msModelId,
                                                  /*bSourceToDestination*/ true);
            if (!sFirstPresChildId.isEmpty())
                // It has a presentation child: is that a presentation of a
                // model node?
                sNodeId = navigate(mrLayoutNode, XML_presOf, sFirstPresChildId,
                                   /*bSourceToDestination*/ false);
        }

        if (!sNodeId.isEmpty())
        {
            for (const auto& aCxn : mrLayoutNode.getDiagram().getData()->getConnections())
                if (aCxn.mnType == XML_parOf && aCxn.msSourceId == sNodeId)
                    nCount++;
        }
        else
        {
            // No presentation child is a presentation of a model node: just
            // count presentation children.
            for (const auto& aCxn : mrLayoutNode.getDiagram().getData()->getConnections())
                if (aCxn.mnType == XML_presParOf && aCxn.msSourceId == pPoint->msModelId)
                    nCount++;
        }
    }
    return nCount;
}

bool ConditionAtom::getDecision() const
{
    if (mIsElse)
        return true;

    switch (maCond.mnFunc)
    {
    case XML_var:
    {
        const dgm::Point* pPoint = getPresNode();
        if (!pPoint)
            break;

        if (maCond.mnArg == XML_dir)
            return compareResult(maCond.mnOp, pPoint->mnDirection, maCond.mnVal);
        else if (maCond.mnArg == XML_hierBranch)
        {
            sal_Int32 nHierarchyBranch = pPoint->moHierarchyBranch.get(XML_std);
            if (!pPoint->moHierarchyBranch.has())
            {
                // If <dgm:hierBranch> is missing in the current presentation
                // point, ask the parent.
                OUString aParent = navigate(mrLayoutNode, XML_presParOf, pPoint->msModelId,
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
        return compareResult(maCond.mnOp, getNodeCount(), maCond.msVal.toInt32());

    case XML_maxDepth:
        return compareResult(maCond.mnOp, mrLayoutNode.getDiagram().getData()->getMaxDepth(), maCond.msVal.toInt32());

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

void ConstraintAtom::parseConstraint(std::vector<Constraint>& rConstraints,
                                     bool bRequireForName) const
{
    // Whitelist for cases where empty forName is handled.
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

void AlgAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void AlgAtom::layoutShape( const ShapePtr& rShape,
                           const std::vector<Constraint>& rOwnConstraints )
{
    // Algorithm result may depend on the parent constraints as well.
    std::vector<Constraint> aMergedConstraints;
    const LayoutNode* pParent = getLayoutNode().getParentLayoutNode();
    if (pParent)
    {
        for (const auto& pChild : pParent->getChildren())
        {
            auto pConstraintAtom = dynamic_cast<ConstraintAtom*>(pChild.get());
            if (pConstraintAtom)
                pConstraintAtom->parseConstraint(aMergedConstraints, /*bRequireForName=*/true);
        }
    }
    aMergedConstraints.insert(aMergedConstraints.end(), rOwnConstraints.begin(),
                              rOwnConstraints.end());
    const std::vector<Constraint>& rConstraints = aMergedConstraints;

    switch(mnType)
    {
        case XML_composite:
        {
            // layout shapes using basic constraints

            LayoutPropertyMap aProperties;
            LayoutProperty& rParent = aProperties[""];

            sal_Int32 nParentXOffset = 0;
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
                const LayoutPropertyMap::const_iterator aRef = aProperties.find(rConstr.msRefForName);
                if (aRef != aProperties.end())
                {
                    const LayoutProperty::const_iterator aRefType = aRef->second.find(rConstr.mnRefType);
                    if (aRefType != aRef->second.end())
                        aProperties[rConstr.msForName][rConstr.mnType] = aRefType->second * rConstr.mfFactor;
                    else
                    {
                        // Values are never in EMU, while oox::drawingml::Shape
                        // position and size are always in EMU.
                        double fUnitFactor = 0;
                        if (isFontUnit(rConstr.mnRefType))
                            // Points -> EMU.
                            fUnitFactor = EMU_PER_PT;
                        else
                            // Millimeters -> EMU.
                            fUnitFactor = EMU_PER_HMM * 100;
                        aProperties[rConstr.msForName][rConstr.mnType]
                            = rConstr.mfValue * fUnitFactor;
                    }
                }
            }

            for (auto & aCurrShape : rShape->getChildren())
            {
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
            }
            break;
        }

        case XML_conn:
        {
            if (rShape->getSubType() == XML_conn)
            {
                // There is no shape type "conn", replace it by an arrow based
                // on the direction of the parent linear layout.
                sal_Int32 nType = getConnectorType(pParent);

                rShape->setSubType(nType);
                rShape->getCustomShapeProperties()->setShapePresetType(nType);

                if (nType == XML_bentConnector3)
                {
                    setHierChildConnPosSize(rShape);
                    break;
                }
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
            const sal_Int32 nShapes = rShape->getChildren().size();
            const awt::Size aCenter(rShape->getSize().Width / 2, rShape->getSize().Height / 2);
            const awt::Size aChildSize(rShape->getSize().Width / 5, rShape->getSize().Height / 5);
            const sal_Int32 nRadius = std::min(
                (rShape->getSize().Width - aChildSize.Width) / 2,
                (rShape->getSize().Height - aChildSize.Height) / 2);

            sal_Int32 idx = 0;
            for (auto & aCurrShape : rShape->getChildren())
            {
                const double fAngle = static_cast<double>(idx)*nSpanAngle/nShapes + nStartAngle;
                const awt::Point aCurrPos(
                    aCenter.Width + nRadius*sin(basegfx::deg2rad(fAngle)) - aChildSize.Width/2,
                    aCenter.Height - nRadius*cos(basegfx::deg2rad(fAngle)) - aChildSize.Height/2);

                aCurrShape->setPosition(aCurrPos);
                aCurrShape->setSize(aChildSize);
                aCurrShape->setChildSize(aChildSize);

                if (nRotationPath == XML_alongPath)
                    aCurrShape->setRotation(fAngle * PER_DEGREE);

                idx++;
            }
            break;
        }

        case XML_hierChild:
        case XML_hierRoot:
        {
            // hierRoot is the manager -> employees vertical linear path,
            // hierChild is the first employee -> last employee horizontal
            // linear path.
            const sal_Int32 nDir = mnType == XML_hierRoot ? XML_fromT : XML_fromL;
            if (rShape->getChildren().empty() || rShape->getSize().Width == 0
                || rShape->getSize().Height == 0)
                break;

            sal_Int32 nCount = rShape->getChildren().size();

            if (mnType == XML_hierChild)
            {
                // Connectors should not influence the size of non-connect
                // shapes.
                nCount = std::count_if(
                    rShape->getChildren().begin(), rShape->getChildren().end(),
                    [](const ShapePtr& pShape) { return pShape->getSubType() != XML_conn; });
            }

            // A manager node's height should be independent from if it has
            // assistants and employees, compensate for that.
            bool bTop = mnType == XML_hierRoot && rShape->getInternalName() == "hierRoot1";

            // Add spacing, so connectors have a chance to be visible.
            double fSpace = (nCount > 1 || bTop) ? 0.3 : 0;

            double fHeightScale = 1.0;
            if (mnType == XML_hierRoot && nCount < 3 && bTop)
                fHeightScale = fHeightScale * nCount / 3;

            if (mnType == XML_hierRoot && nCount == 3)
            {
                // Order assistant nodes above employee nodes.
                std::vector<ShapePtr>& rChildren = rShape->getChildren();
                if (!containsDataNodeType(rChildren[1], XML_asst)
                    && containsDataNodeType(rChildren[2], XML_asst))
                    std::swap(rChildren[1], rChildren[2]);
            }

            sal_Int32 nXOffset = 0;
            double fWidthScale = 1.0;
            if (mnType == XML_hierChild)
                calculateHierChildOffsetScale(rShape, pParent, nXOffset, fWidthScale);

            awt::Size aChildSize = rShape->getSize();
            if (nDir == XML_fromT)
            {
                aChildSize.Height /= (nCount + nCount * fSpace);
            }
            else
                aChildSize.Width /= nCount;
            aChildSize.Height *= fHeightScale;
            aChildSize.Width *= fWidthScale;

            awt::Point aChildPos(nXOffset, 0);
            for (auto& pChild : rShape->getChildren())
            {
                pChild->setPosition(aChildPos);
                pChild->setSize(aChildSize);
                pChild->setChildSize(aChildSize);

                if (mnType == XML_hierChild && pChild->getSubType() == XML_conn)
                    // Connectors should not influence the position of
                    // non-connect shapes.
                    continue;

                if (nDir == XML_fromT)
                    aChildPos.Y += aChildSize.Height + aChildSize.Height * fSpace;
                else
                    aChildPos.X += aChildSize.Width;
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

            sal_Int32 nCount = rShape->getChildren().size();
            double fSpace = 0.3;

            // Find out which constraint is relevant for which (internal) name.
            LayoutPropertyMap aProperties;
            for (const auto& rConstraint : rConstraints)
            {
                if (rConstraint.msForName.isEmpty())
                    continue;

                LayoutProperty& rProperty = aProperties[rConstraint.msForName];
                if (rConstraint.mnType == XML_w)
                    rProperty[XML_w] = rShape->getSize().Width * rConstraint.mfFactor;

                // TODO: get values from differently named constraints as well
                if (rConstraint.msForName == "sibTrans" && rConstraint.mnType == XML_w)
                    fSpace = rConstraint.mfFactor;
            }

            awt::Size aChildSize = rShape->getSize();
            if (nDir == XML_fromL || nDir == XML_fromR)
                aChildSize.Width /= (nCount + (nCount-1)*fSpace);
            else if (nDir == XML_fromT || nDir == XML_fromB)
                aChildSize.Height /= (nCount + (nCount-1)*fSpace);

            awt::Point aCurrPos(0, 0);
            if (nIncX == -1)
                aCurrPos.X = rShape->getSize().Width - aChildSize.Width;
            if (nIncY == -1)
                aCurrPos.Y = rShape->getSize().Height - aChildSize.Height;

            // See if children requested more than 100% space in total: scale
            // down in that case.
            sal_Int32 nTotalWidth = 0;
            bool bSpaceFromConstraints = false;
            for (auto & aCurrShape : rShape->getChildren())
            {
                oox::OptValue<sal_Int32> oWidth
                    = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);

                awt::Size aSize = aChildSize;
                if (oWidth.has())
                {
                    aSize.Width = oWidth.get();
                    bSpaceFromConstraints = true;
                }
                if (nDir == XML_fromL || nDir == XML_fromR)
                    nTotalWidth += aSize.Width;
            }

            double fWidthScale = 1.0;
            if (nTotalWidth > rShape->getSize().Width && nTotalWidth)
            {
                fWidthScale = rShape->getSize().Width;
                fWidthScale /= nTotalWidth;
            }

            // Don't add automatic space if we take space from constraints.
            if (bSpaceFromConstraints)
                fSpace = 0;

            for (auto& aCurrShape : rShape->getChildren())
            {
                // Extract properties relevant for this shape from constraints.
                oox::OptValue<sal_Int32> oWidth
                    = findProperty(aProperties, aCurrShape->getInternalName(), XML_w);

                aCurrShape->setPosition(aCurrPos);

                awt::Size aSize = aChildSize;
                if (oWidth.has())
                    aSize.Width = oWidth.get();
                aSize.Width *= fWidthScale;
                aCurrShape->setSize(aSize);

                aCurrShape->setChildSize(aSize);
                aCurrPos.X += nIncX * (aSize.Width + fSpace*aSize.Width);
                aCurrPos.Y += nIncY * (aChildSize.Height + fSpace*aChildSize.Height);
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
                aCurrPos.X -=  aChildSize.Height/(nCount-1);
                aChildSize.Width += aChildSize.Height;
                aCurrShape->setSize(aChildSize);
                aCurrShape->setChildSize(aChildSize);
                aCurrPos.Y += (aChildSize.Height);
            }
            break;
        }

        case XML_snake:
        {
            // find optimal grid to layout children that have fixed aspect ratio

            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

            // Parse constraints, only self spacing from height as a start.
            double fSpaceFromConstraint = 0;
            for (const auto& rConstr : rConstraints)
            {
                if (rConstr.mnRefType == XML_h)
                {
                    if (rConstr.mnType == XML_sp && rConstr.msForName.isEmpty())
                        fSpaceFromConstraint = rConstr.mfFactor;
                }
            }
            bool bSpaceFromConstraints = fSpaceFromConstraint != 0;

            const sal_Int32 nDir = maMap.count(XML_grDir) ? maMap.find(XML_grDir)->second : XML_tL;
            sal_Int32 nIncX = 1;
            sal_Int32 nIncY = 1;
            switch (nDir)
            {
                case XML_tL: nIncX =  1; nIncY =  1; break;
                case XML_tR: nIncX = -1; nIncY =  1; break;
                case XML_bL: nIncX =  1; nIncY = -1; break;
                case XML_bR: nIncX = -1; nIncY = -1; break;
            }

            sal_Int32 nCount = rShape->getChildren().size();
            // Defaults in case not provided by constraints.
            double fSpace = bSpaceFromConstraints ? fSpaceFromConstraint : 0.3;
            double fAspectRatio = 0.54; // diagram should not spill outside, earlier it was 0.6

            sal_Int32 nCol = 1;
            sal_Int32 nRow = 1;
            double fChildAspectRatio = rShape->getChildren()[0]->getAspectRatio();
            if (nCount <= fChildAspectRatio)
                // Child aspect ratio request (width/height) is N, and we have at most N shapes.
                // This means we don't need multiple columns.
                nRow = nCount;
            else
            {
                for ( ; nRow<nCount; nRow++)
                {
                    nCol = (nCount+nRow-1) / nRow;
                    const double fShapeHeight = rShape->getSize().Height;
                    const double fShapeWidth = rShape->getSize().Width;
                    if ((fShapeHeight / nCol) / (fShapeWidth / nRow) >= fAspectRatio)
                        break;
                }
            }

            SAL_INFO("oox.drawingml", "Snake layout grid: " << nCol << "x" << nRow);

            sal_Int32 nWidth = rShape->getSize().Width / (nCol + (nCol-1)*fSpace);
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
                sal_Int32 nHeight
                    = rShape->getSize().Height / (nRow + (nRow + nNumSpaces) * fSpace);

                if (fChildAspectRatio > 1)
                {
                    // Shrink width if the aspect ratio requires it.
                    nWidth = std::min(rShape->getSize().Width,
                                      static_cast<sal_Int32>(nHeight * fChildAspectRatio));
                    aChildSize = awt::Size(nWidth, nHeight);
                }
            }

            awt::Point aCurrPos(0, 0);
            if (nIncX == -1)
                aCurrPos.X = rShape->getSize().Width - aChildSize.Width;
            if (nIncY == -1)
                aCurrPos.Y = rShape->getSize().Height - aChildSize.Height;
            else if (bSpaceFromConstraints)
                // Initial vertical offset to have upper spacing (outside, so double amount).
                aCurrPos.Y = aChildSize.Height * fSpace * 2;

            sal_Int32 nStartX = aCurrPos.X;
            sal_Int32 nColIdx = 0,index = 0;

            const sal_Int32 aContDir = maMap.count(XML_contDir) ? maMap.find(XML_contDir)->second : XML_sameDir;

            switch(aContDir)
            {
                case XML_sameDir:
                for (auto & aCurrShape : rShape->getChildren())
                {
                    aCurrShape->setPosition(aCurrPos);
                    aCurrShape->setSize(aChildSize);
                    aCurrShape->setChildSize(aChildSize);

                    index++; // counts index of child, helpful for positioning.

                    if(index%nCol==0 || ((index/nCol)+1)!=nRow)
                        aCurrPos.X += nIncX * (aChildSize.Width + fSpace*aChildSize.Width);

                    if(++nColIdx == nCol) // condition for next row
                    {
                        // if last row, then position children according to number of shapes.
                        if((index+1)%nCol!=0 && (index+1)>=3 && ((index+1)/nCol+1)==nRow && nCount!=nRow*nCol)
                            // position first child of last row
                            aCurrPos.X = nStartX + (nIncX * (aChildSize.Width + fSpace*aChildSize.Width))/2;
                        else
                            // if not last row, positions first child of that row
                            aCurrPos.X = nStartX;
                        aCurrPos.Y += nIncY * (aChildSize.Height + fSpace*aChildSize.Height);
                        nColIdx = 0;
                    }

                    // positions children in the last row.
                    if(index%nCol!=0 && index>=3 && ((index/nCol)+1)==nRow)
                        aCurrPos.X += (nIncX * (aChildSize.Width + fSpace*aChildSize.Width));
                }
                break;
                case XML_revDir:
                for (auto & aCurrShape : rShape->getChildren())
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

                    if((index%nCol==0 || ((index/nCol)+1)!=nRow) && ((index/nCol)+1)%2!=0)
                        aCurrPos.X +=  (aChildSize.Width + fSpace*aChildSize.Width);
                    else if( index%nCol!=0 && ((index/nCol)+1)!=nRow) // child other than placed at last column
                        aCurrPos.X -= (aChildSize.Width + fSpace*aChildSize.Width);

                    if(++nColIdx == nCol) // condition for next row
                    {
                        // if last row, then position children according to number of shapes.
                        if((index+1)%nCol!=0 && (index+1)>=4 && ((index+1)/nCol+1)==nRow && nCount!=nRow*nCol && ((index/nCol)+1)%2==0)
                            // position first child of last row
                            aCurrPos.X -= aChildSize.Width*3/2;
                        else if((index+1)%nCol!=0 && (index+1)>=4 && ((index+1)/nCol+1)==nRow && nCount!=nRow*nCol && ((index/nCol)+1)%2!=0)
                            aCurrPos.X = nStartX + (nIncX * (aChildSize.Width + fSpace*aChildSize.Width))/2;
                        else if(((index/nCol)+1)%2!=0)
                            aCurrPos.X = nStartX;

                        aCurrPos.Y += nIncY * (aChildSize.Height + fSpace*aChildSize.Height);
                        nColIdx = 0;
                    }

                    // positions children in the last row.
                    if(index%nCol!=0 && index>=3 && ((index/nCol)+1)==nRow && ((index/nCol)+1)%2==0)
                        //if row%2=0 then start from left else
                        aCurrPos.X -= (nIncX * (aChildSize.Width + fSpace*aChildSize.Width));
                    else if(index%nCol!=0 && index>=3 && ((index/nCol)+1)==nRow && ((index/nCol)+1)%2!=0)
                        // start from right
                        aCurrPos.X += (nIncX * (aChildSize.Width + fSpace*aChildSize.Width));
                }
                break;
            }
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
            }

            // TODO: adjust text size to fit shape
            TextBodyPtr pTextBody = rShape->getTextBody();
            if (!pTextBody ||
                pTextBody->getParagraphs().empty() ||
                pTextBody->getParagraphs().front()->getRuns().empty())
            {
                break;
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

            ParamMap::const_iterator aBulletLvl = maMap.find(XML_stBulletLvl);
            int nStartBulletsAtLevel = 0;
            if (aBulletLvl != maMap.end())
            {
                nBaseLevel -= aBulletLvl->second;
                nStartBulletsAtLevel = aBulletLvl->second;
            }

            for (auto & aParagraph : pTextBody->getParagraphs())
            {
                sal_Int32 nLevel = aParagraph->getProperties().getLevel();
                aParagraph->getProperties().setLevel(nLevel - nBaseLevel);
                if (nStartBulletsAtLevel > 0 && nLevel >= nStartBulletsAtLevel)
                {
                    // It is not possible to change the bullet style for text.
                    sal_Int32 nLeftMargin = 285750 * (nLevel - nStartBulletsAtLevel) / EMU_PER_HMM;
                    aParagraph->getProperties().getParaLeftMargin() = nLeftMargin;
                    aParagraph->getProperties().getFirstLineIndentation() = -285750 / EMU_PER_HMM;
                    OUString aBulletChar = OUString::fromUtf8(u8"•");
                    aParagraph->getProperties().getBulletList().setBulletChar(aBulletChar);
                    aParagraph->getProperties().getBulletList().setSuffixNone();
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
            else if (std::all_of(pTextBody->getParagraphs().begin(), pTextBody->getParagraphs().end(),
                [](const std::shared_ptr<TextParagraph>& aParagraph) { return aParagraph->getProperties().getLevel() == 0; }))
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

bool LayoutNode::setupShape( const ShapePtr& rShape, const dgm::Point* pPresNode ) const
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
                    pTextBody.reset( new TextBody() );

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
            const DiagramColor& rColor=aColor->second;
            if( rColor.maFillColor.isUsed() )
                rShape->getShapeStyleRefs()[XML_fillRef].maPhClr = rColor.maFillColor;
            if( rColor.maLineColor.isUsed() )
                rShape->getShapeStyleRefs()[XML_lnRef].maPhClr = rColor.maLineColor;
            if( rColor.maEffectColor.isUsed() )
                rShape->getShapeStyleRefs()[XML_effectRef].maPhClr = rColor.maEffectColor;
            if( rColor.maTextFillColor.isUsed() )
                rShape->getShapeStyleRefs()[XML_fontRef].maPhClr = rColor.maTextFillColor;
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

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
