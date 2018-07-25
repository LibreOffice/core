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

#include <oox/helper/attributelist.hxx>
#include <oox/token/properties.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/lineproperties.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <drawingml/textrun.hxx>
#include <drawingml/customshapeproperties.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

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

ForEachAtom::ForEachAtom(const LayoutNode& rLayoutNode, const Reference< XFastAttributeList >& xAttributes) :
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

ConditionAtom::ConditionAtom(const LayoutNode& rLayoutNode, bool isElse, const Reference< XFastAttributeList >& xAttributes) :
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

sal_Int32 ConditionAtom::getNodeCount() const
{
    sal_Int32 nCount = 0;
    const dgm::Point* pPoint = getPresNode();
    if (pPoint)
    {
        OUString sNodeId = "";

        for (const auto& aCxn : mrLayoutNode.getDiagram().getData()->getConnections())
            if (aCxn.mnType == XML_presOf && aCxn.msDestId == pPoint->msModelId)
                sNodeId = aCxn.msSourceId;

        if (!sNodeId.isEmpty())
        {
            for (const auto& aCxn : mrLayoutNode.getDiagram().getData()->getConnections())
                if (aCxn.mnType == XML_parOf && aCxn.msSourceId == sNodeId)
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
        if (pPoint && maCond.mnArg == XML_dir)
            return compareResult(maCond.mnOp, pPoint->mnDirection, maCond.mnVal);
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

void ConstraintAtom::parseConstraint(std::vector<Constraint>& rConstraints) const
{
    // accepting only basic equality constraints
    if (!maConstraint.msForName.isEmpty() &&
        (maConstraint.mnOperator == XML_none || maConstraint.mnOperator == XML_equ) &&
        maConstraint.mnType != XML_none &&
        maConstraint.mfValue == 0)
    {
        rConstraints.push_back(maConstraint);
    }
}

void AlgAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

void AlgAtom::layoutShape( const ShapePtr& rShape,
                           const std::vector<Constraint>& rConstraints ) const
{
    switch(mnType)
    {
        case XML_composite:
        {
            // layout shapes using basic constraints

            LayoutPropertyMap aProperties;
            LayoutProperty& rParent = aProperties[""];
            rParent[XML_w] = rShape->getSize().Width;
            rParent[XML_h] = rShape->getSize().Height;
            rParent[XML_l] = 0;
            rParent[XML_t] = 0;
            rParent[XML_r] = rShape->getSize().Width;
            rParent[XML_b] = rShape->getSize().Height;

            const double ar = maMap.count(XML_ar) ? maMap.find(XML_ar)->second : 0;

            if(ar)
            {
                if(rParent[XML_w]>rParent[XML_h])
                    rParent[XML_w] = ar*rParent[XML_h];
                else
                    rParent[XML_h] = ar*rParent[XML_w];
            }

            for (const auto & rConstr : rConstraints)
            {
                const LayoutPropertyMap::const_iterator aRef = aProperties.find(rConstr.msRefForName);
                if(rConstr.mfValue)
                   aProperties[rConstr.msForName][rConstr.mnType] = rConstr.mfValue;
                else if (aRef != aProperties.end())
                {
                    const LayoutProperty::const_iterator aRefType = aRef->second.find(rConstr.mnRefType);
                    if (aRefType != aRef->second.end())
                        aProperties[rConstr.msForName][rConstr.mnType] = aRefType->second * rConstr.mfFactor;
                    else
                        aProperties[rConstr.msForName][rConstr.mnType] = 0;
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
                        aSize.Width = it->second;
                    if ( (it = rProp.find(XML_h)) != rProp.end() )
                        aSize.Height = it->second;

                    if ( (it = rProp.find(XML_l)) != rProp.end() )
                        aPos.X = it->second;
                    else if ( (it = rProp.find(XML_ctrX)) != rProp.end() )
                        aPos.X = it->second - aSize.Width/2;

                    if ( (it = rProp.find(XML_t)) != rProp.end())
                        aPos.Y = it->second;
                    else if ( (it = rProp.find(XML_ctrY)) != rProp.end() )
                        aPos.Y = it->second - aSize.Height/2;

                    if ( (it = rProp.find(XML_l)) != rProp.end() && (it2 = rProp.find(XML_r)) != rProp.end() )
                        aSize.Width = it2->second - it->second;
                    if ( (it = rProp.find(XML_t)) != rProp.end() && (it2 = rProp.find(XML_b)) != rProp.end() )
                        aSize.Height = it2->second - it->second;

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
            break;

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
                    aCenter.Width + nRadius*sin(fAngle*F_PI180) - aChildSize.Width/2,
                    aCenter.Height - nRadius*cos(fAngle*F_PI180) - aChildSize.Height/2);

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
            break;

        case XML_lin:
        {
            // spread children evenly across one axis, stretch across second

            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

            const sal_Int32 nDir = maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromL;
            const sal_Int32 nIncX = nDir==XML_fromL ? 1 : (nDir==XML_fromR ? -1 : 0);
            const sal_Int32 nIncY = nDir==XML_fromT ? 1 : (nDir==XML_fromB ? -1 : 0);

            // TODO: get values from constraints
            sal_Int32 nCount = rShape->getChildren().size();
            double fSpace = 0.3;

            awt::Size aChildSize = rShape->getSize();

            aChildSize.Width /= (nCount + (nCount-1)*fSpace);
            aChildSize.Height /= (nCount + (nCount-1)*fSpace);

            awt::Point aCurrPos(0, 0);
            if (nIncX == -1)
                aCurrPos.X = rShape->getSize().Width - aChildSize.Width;
            if (nIncY == -1)
                aCurrPos.Y = rShape->getSize().Height - aChildSize.Height;

            for (auto & aCurrShape : rShape->getChildren())
            {
                aCurrShape->setPosition(aCurrPos);
                aCurrShape->setSize(aChildSize);
                aCurrShape->setChildSize(aChildSize);
                aCurrPos.X += nIncX * (aChildSize.Width + fSpace*aChildSize.Width);
                aCurrPos.Y += nIncY * (aChildSize.Height + fSpace*aChildSize.Height);
            }
            break;
        }

        case XML_pyra:
        {
            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

            const sal_Int32 nDir = maMap.count(XML_linDir) ? maMap.find(XML_linDir)->second : XML_fromT;
            // const sal_Int32 npyraAcctPos = maMap.count(XML_pyraAcctPos) ? maMap.find(XML_pyraAcctPos)->second : XML_bef;
            // const sal_Int32 ntxDir = maMap.count(XML_txDir) ? maMap.find(XML_txDir)->second : XML_fromT;
            // const sal_Int32 npyraLvlNode = maMap.count(XML_pyraLvlNode) ? maMap.find(XML_pyraLvlNode)->second : XML_level;
            // uncomment when use in code.

            sal_Int32 nCount = rShape->getChildren().size();
            double fAspectRatio = 0.33;

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

            if (nDir == XML_fromT)
                    rShape->setRotation(180* PER_DEGREE);
            break;
        }

        case XML_snake:
        {
            // find optimal grid to layout children that have fixed aspect ratio

            if (rShape->getChildren().empty() || rShape->getSize().Width == 0 || rShape->getSize().Height == 0)
                break;

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

            // TODO: get values from constraints
            sal_Int32 nCount = rShape->getChildren().size();
            double fSpace = 0.3;
            double fAspectRatio = 0.54; // diagram should not spill outside, earlier it was 0.6

            sal_Int32 nCol = 1;
            sal_Int32 nRow = 1;
            for ( ; nRow<nCount; nRow++)
            {
                nCol = (nCount+nRow-1) / nRow;
                const double fShapeHeight = rShape->getSize().Height;
                const double fShapeWidth = rShape->getSize().Width;
                if ((fShapeHeight / nCol) / (fShapeWidth / nRow) >= fAspectRatio)
                    break;
            }

            SAL_INFO("oox.drawingml", "Snake layout grid: " << nCol << "x" << nRow);

            sal_Int32 nWidth = rShape->getSize().Width / (nCol + (nCol-1)*fSpace);
            const awt::Size aChildSize(nWidth, nWidth * fAspectRatio);

            awt::Point aCurrPos(0, 0);
            if (nIncX == -1)
                aCurrPos.X = rShape->getSize().Width - aChildSize.Width;
            if (nIncY == -1)
                aCurrPos.Y = rShape->getSize().Height - aChildSize.Height;

            sal_Int32 nStartX = aCurrPos.X;
            sal_Int32 nColIdx = 0,index = 0;

            sal_Int32 num = rShape->getChildren().size();

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
                        if((index+1)%nCol!=0 && (index+1)>=3 && ((index+1)/nCol+1)==nRow && num!=nRow*nCol)
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
                        if((index+1)%nCol!=0 && (index+1)>=4 && ((index+1)/nCol+1)==nRow && num!=nRow*nCol && ((index/nCol)+1)%2==0)
                            // position first child of last row
                            aCurrPos.X -= aChildSize.Width*3/2;
                        else if((index+1)%nCol!=0 && (index+1)>=4 && ((index+1)/nCol+1)==nRow && num!=nRow*nCol && ((index/nCol)+1)%2!=0)
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
            // TODO: adjust text size to fit shape
            TextBodyPtr pTextBody = rShape->getTextBody();
            if (!pTextBody ||
                pTextBody->getParagraphs().empty() ||
                pTextBody->getParagraphs().front()->getRuns().empty())
            {
                break;
            }

            const sal_Int32 nautoTxRot = maMap.count(XML_autoTxRot) ? maMap.find(XML_autoTxRot)->second : XML_upr;

            switch(nautoTxRot)
            {
                case XML_upr:
                {
                    if (rShape->getRotation())
                        pTextBody->getTextProperties().moRotation = -F_PI180*90*rShape->getRotation();
                }
                break;
                case XML_grav:
                {
                    if (rShape->getRotation()==90*F_PI180 || rShape->getRotation()==180*F_PI180)
                        pTextBody->getTextProperties().moRotation = 180*F_PI180;
                }
                break;
                case XML_none:
                break;
            }

            // text centered vertically by default
            pTextBody->getTextProperties().meVA = css::drawing::TextVerticalAdjust_CENTER;
            pTextBody->getTextProperties().maPropertyMap.setProperty(PROP_TextVerticalAdjust, css::drawing::TextVerticalAdjust_CENTER);

            // normalize list level
            sal_Int32 nBaseLevel = pTextBody->getParagraphs().front()->getProperties().getLevel();
            for (auto & aParagraph : pTextBody->getParagraphs())
            {
                if (aParagraph->getProperties().getLevel() < nBaseLevel)
                    nBaseLevel = aParagraph->getProperties().getLevel();
            }

            ParamMap::const_iterator aBulletLvl = maMap.find(XML_stBulletLvl);
            if (aBulletLvl != maMap.end())
                nBaseLevel -= aBulletLvl->second;

            for (auto & aParagraph : pTextBody->getParagraphs())
            {
                sal_Int32 nLevel = aParagraph->getProperties().getLevel();
                aParagraph->getProperties().setLevel(nLevel - nBaseLevel);
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
        DiagramData::StringMap::value_type::second_type::const_iterator aVecIter=aNodeName->second.begin();
        const DiagramData::StringMap::value_type::second_type::const_iterator aVecEnd=aNodeName->second.end();
        while( aVecIter != aVecEnd )
        {
            DiagramData::PointNameMap& rMap = mrDgm.getData()->getPointNameMap();
            DiagramData::PointNameMap::const_iterator aDataNode2 = rMap.find(aVecIter->first);
            if (aDataNode2 == rMap.end())
            {
                //busted, skip it
                ++aVecIter;
                continue;
            }

            if( aVecIter->second == 0 )
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

                TextParagraph& rPara=pTextBody->addParagraph();
                if( aVecIter->second != -1 )
                    rPara.getProperties().setLevel(aVecIter->second);

                rPara.addRun(
                    aDataNode2->second->mpShape->getTextBody()->getParagraphs().front()->getRuns().front());
                rPara.getProperties().apply(
                    aDataNode2->second->mpShape->getTextBody()->getParagraphs().front()->getProperties());
            }

            ++aVecIter;
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

void ShapeAtom::accept( LayoutAtomVisitor& rVisitor )
{
    rVisitor.visit(*this);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
