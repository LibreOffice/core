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

#include <unordered_set>
#include <algorithm>
#include <fstream>

#include <svx/diagram/datamodel_svx.hxx>
#include <svx/svdobj.hxx>
#include <svx/svditer.hxx>
#include <comphelper/xmltools.hxx>
#include <sal/log.hxx>
#include <utility>
#include <sax/fastattribs.hxx>
#include <com/sun/star/text/XText.hpp>

using namespace ::oox;
using namespace ::com::sun::star;

namespace svx::diagram {

void addTypeConstantToFastAttributeList(TypeConstant aTypeConstant, rtl::Reference<sax_fastparser::FastAttributeList>& rAttributeList)
{
    if (TypeConstant::XML_none != aTypeConstant)
    {
        switch (aTypeConstant)
        {
            // *CAUTION!* here '::XML_type' is *not* the same as 'XML_type' which would
            // namespace expand to oox::XML_type as in enum TypeConstant definitions (!)
            case TypeConstant::XML_type: rAttributeList->add(::XML_type, "Type"); break;
            case TypeConstant::XML_asst: rAttributeList->add(::XML_type, "asst"); break;
            case TypeConstant::XML_doc: rAttributeList->add(::XML_type, "doc"); break;
            case TypeConstant::XML_node: /* XML_node is default, no need to write */ break;
            case TypeConstant::XML_norm: rAttributeList->add(::XML_type, "norm"); break;
            case TypeConstant::XML_parOf: rAttributeList->add(::XML_type, "parOf"); break;
            case TypeConstant::XML_parTrans: rAttributeList->add(::XML_type, "parTrans"); break;
            case TypeConstant::XML_pres: rAttributeList->add(::XML_type, "pres"); break;
            case TypeConstant::XML_presOf: rAttributeList->add(::XML_type, "presOf"); break;
            case TypeConstant::XML_presParOf: rAttributeList->add(::XML_type, "presParOf"); break;
            case TypeConstant::XML_rel: rAttributeList->add(::XML_type, "rel"); break;
            case TypeConstant::XML_sibTrans: rAttributeList->add(::XML_type, "sibTrans"); break;
            default: break; // XML_none
        }
    }
}

Connection::Connection()
: mnXMLType( XML_none )
, mnSourceOrder( 0 )
, mnDestOrder( 0 )
{
}

void Connection::writeDiagramData(sax_fastparser::FSHelperPtr& rTarget)
{
    if (!rTarget)
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(sax_fastparser::FastSerializerHelper::createAttrList());

    addTypeConstantToFastAttributeList(mnXMLType, pAttributeList);
    if (!msModelId.isEmpty()) pAttributeList->add(XML_modelId, msModelId);
    if (!msSourceId.isEmpty()) pAttributeList->add(XML_srcId, msSourceId);
    if (!msDestId.isEmpty()) pAttributeList->add(XML_destId, msDestId);
    if (!msPresId.isEmpty()) pAttributeList->add(XML_presId, msPresId);
    if (!msSibTransId.isEmpty()) pAttributeList->add(XML_sibTransId, msSibTransId);
    if (!msParTransId.isEmpty()) pAttributeList->add(XML_parTransId, msParTransId);
    if (0 != mnSourceOrder) pAttributeList->add(XML_srcOrd, OUString::number(mnSourceOrder));
    if (0 != mnDestOrder) pAttributeList->add(XML_destOrd, OUString::number(mnDestOrder));

    rTarget->singleElementNS(XML_dgm, XML_cxn, pAttributeList);
}

Point::Point()
: mnXMLType(XML_none)
, mnMaxChildren(-1)
, mnPreferredChildren(-1)
, mnDirection(XML_norm)
, mnCustomAngle(-1)
, mnPercentageNeighbourWidth(-1)
, mnPercentageNeighbourHeight(-1)
, mnPercentageOwnWidth(-1)
, mnPercentageOwnHeight(-1)
, mnIncludeAngleScale(-1)
, mnRadiusScale(-1)
, mnWidthScale(-1)
, mnHeightScale(-1)
, mnWidthOverride(-1)
, mnHeightOverride(-1)
, mnLayoutStyleCount(-1)
, mnLayoutStyleIndex(-1)
, mbOrgChartEnabled(false)
, mbBulletEnabled(false)
, mbCoherent3DOffset(false)
, mbCustomHorizontalFlip(false)
, mbCustomVerticalFlip(false)
, mbCustomText(false)
, mbIsPlaceholder(false)
{
}

void Point::writeDiagramData_data(sax_fastparser::FSHelperPtr& rTarget)
{
    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(sax_fastparser::FastSerializerHelper::createAttrList());

    if (!msColorTransformCategoryId.isEmpty()) pAttributeList->add(XML_csCatId, msColorTransformCategoryId);
    if (!msColorTransformTypeId.isEmpty()) pAttributeList->add(XML_csTypeId, msColorTransformTypeId);
    if (!msLayoutCategoryId.isEmpty()) pAttributeList->add(XML_loCatId, msLayoutCategoryId);
    if (!msLayoutTypeId.isEmpty()) pAttributeList->add(XML_loTypeId, msLayoutTypeId);
    if (!msPlaceholderText.isEmpty()) pAttributeList->add(XML_phldrT, msPlaceholderText);
    if (!msPresentationAssociationId.isEmpty()) pAttributeList->add(XML_presAssocID, msPresentationAssociationId);
    if (!msPresentationLayoutName.isEmpty()) pAttributeList->add(XML_presName, msPresentationLayoutName);
    if (!msPresentationLayoutStyleLabel.isEmpty()) pAttributeList->add(XML_presStyleLbl, msPresentationLayoutStyleLabel);
    if (!msQuickStyleCategoryId.isEmpty()) pAttributeList->add(XML_qsCatId, msQuickStyleCategoryId);
    if (!msQuickStyleTypeId.isEmpty()) pAttributeList->add(XML_qsTypeId, msQuickStyleTypeId);

    if (-1 != mnCustomAngle) pAttributeList->add(XML_custAng, OUString::number(mnCustomAngle));
    if (-1 != mnPercentageNeighbourWidth) pAttributeList->add(XML_custLinFactNeighborX, OUString::number(mnPercentageNeighbourWidth));
    if (-1 != mnPercentageNeighbourHeight) pAttributeList->add(XML_custLinFactNeighborY, OUString::number(mnPercentageNeighbourHeight));
    if (-1 != mnPercentageOwnWidth) pAttributeList->add(XML_custLinFactX, OUString::number(mnPercentageOwnWidth));
    if (-1 != mnPercentageOwnHeight) pAttributeList->add(XML_custLinFactY, OUString::number(mnPercentageOwnHeight));
    if (-1 != mnIncludeAngleScale) pAttributeList->add(XML_custRadScaleInc, OUString::number(mnIncludeAngleScale));
    if (-1 != mnRadiusScale) pAttributeList->add(XML_custRadScaleRad, OUString::number(mnRadiusScale));
    if (-1 != mnWidthScale) pAttributeList->add(XML_custScaleX, OUString::number(mnWidthScale));
    if (-1 != mnHeightScale) pAttributeList->add(XML_custScaleY, OUString::number(mnHeightScale));
    if (-1 != mnWidthOverride) pAttributeList->add(XML_custSzX, OUString::number(mnWidthOverride));
    if (-1 != mnHeightOverride) pAttributeList->add(XML_custSzY, OUString::number(mnHeightOverride));
    if (-1 != mnLayoutStyleCount) pAttributeList->add(XML_presStyleCnt, OUString::number(mnLayoutStyleCount));
    if (-1 != mnLayoutStyleIndex) pAttributeList->add(XML_presStyleIdx, OUString::number(mnLayoutStyleIndex));

    static constexpr OUString aStrTrue = u"1"_ustr; // this uses "1", not "true"
    if (true == mbCoherent3DOffset) pAttributeList->add(XML_coherent3DOff, aStrTrue);
    if (true == mbCustomHorizontalFlip) pAttributeList->add(XML_custFlipHor, aStrTrue);
    if (true == mbCustomVerticalFlip) pAttributeList->add(XML_custFlipVert, aStrTrue);
    if (true == mbCustomText) pAttributeList->add(XML_custT, aStrTrue);
    if (true == mbIsPlaceholder) pAttributeList->add(XML_phldr, aStrTrue);

    const bool bNeed_presLayoutVars(mbBulletEnabled
        || -1 != mnMaxChildren
        || -1 != mnPreferredChildren
        || XML_norm != mnDirection
        || moHierarchyBranch.has_value()
        || mbOrgChartEnabled
        || !msResizeHandles.isEmpty());

    if (bNeed_presLayoutVars)
    {
        rTarget->startElementNS(XML_dgm, XML_prSet, pAttributeList);
        rTarget->startElementNS(XML_dgm, XML_presLayoutVars);

        if (mbBulletEnabled)
            rTarget->singleElementNS(XML_dgm, XML_bulletEnabled, XML_val, aStrTrue);

        if (-1 != mnMaxChildren)
            rTarget->singleElementNS(XML_dgm, XML_chMax, XML_val, OUString::number(mnMaxChildren));

        if (-1 != mnPreferredChildren)
            rTarget->singleElementNS(XML_dgm, XML_chPref, XML_val, OUString::number(mnPreferredChildren));

        if (XML_norm != mnDirection)
            rTarget->singleElementNS(XML_dgm, XML_dir, XML_val, OString::number(mnDirection));

        if (moHierarchyBranch.has_value())
            rTarget->singleElementNS(XML_dgm, XML_hierBranch, XML_val, OString::number(moHierarchyBranch.value()));

        if (mbOrgChartEnabled)
            rTarget->singleElementNS(XML_dgm, XML_orgChart, XML_val, aStrTrue);

        if (!msResizeHandles.isEmpty())
            rTarget->singleElementNS(XML_dgm, XML_resizeHandles, XML_val, msResizeHandles);

        rTarget->endElementNS(XML_dgm, XML_presLayoutVars);
        rTarget->endElementNS(XML_dgm, XML_prSet);
    }
    else
        rTarget->singleElementNS(XML_dgm, XML_prSet, pAttributeList);

    rTarget->singleElementNS(XML_dgm, XML_spPr);
}

DiagramData_svx::DiagramData_svx()
{
}

DiagramData_svx::~DiagramData_svx()
{
}

const Point* DiagramData_svx::getRootPoint() const
{
    for (const auto & aCurrPoint : maPoints)
        if (aCurrPoint.mnXMLType == TypeConstant::XML_doc)
            return &aCurrPoint;

    SAL_WARN("svx.diagram", "No root point");
    return nullptr;
}

OUString DiagramData_svx::getDiagramString(const css::uno::Reference<css::drawing::XShape>& rRootShape) const
{
    OUStringBuffer aBuf;
    const Point* pPoint = getRootPoint();
    getDiagramChildrenString(aBuf, pPoint, 0, rRootShape);
    return aBuf.makeStringAndClear();
}

DomMapFlags DiagramData_svx::removeDiagramNode(const OUString& rNodeId)
{
    DomMapFlags aRetval;

    // check if it doesn't have children
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_parOf && aCxn.msSourceId == rNodeId)
        {
            SAL_WARN("svx.diagram", "Node has children - can't be removed");
            return aRetval;
        }

    Connection aParCxn;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_parOf && aCxn.msDestId == rNodeId)
            aParCxn = aCxn;

    std::unordered_set<OUString> aIdsToRemove;
    aIdsToRemove.insert(rNodeId);
    if (!aParCxn.msParTransId.isEmpty())
        aIdsToRemove.insert(aParCxn.msParTransId);
    if (!aParCxn.msSibTransId.isEmpty())
        aIdsToRemove.insert(aParCxn.msSibTransId);

    for (const Point& rPoint : maPoints)
        if (aIdsToRemove.count(rPoint.msPresentationAssociationId))
            aIdsToRemove.insert(rPoint.msModelId);

    // insert also transition nodes
    for (const auto& aCxn : maConnections)
        if (aIdsToRemove.count(aCxn.msSourceId) || aIdsToRemove.count(aCxn.msDestId))
            if (!aCxn.msPresId.isEmpty())
                aIdsToRemove.insert(aCxn.msPresId);

    // remove connections
    std::erase_if(maConnections,
                                       [&aIdsToRemove](const Connection& rCxn) {
                                           return aIdsToRemove.count(rCxn.msSourceId) || aIdsToRemove.count(rCxn.msDestId);
                                       });

    // remove data and presentation nodes
    std::erase_if(maPoints,
                                  [&aIdsToRemove](const Point& rPoint) {
                                      return aIdsToRemove.count(rPoint.msModelId);
                                  });

    // TODO: fix source/dest order

    // prepare retval, OOXData and OOXLayout is changed
    aRetval.push_back(DomMapFlag::OOXData);
    // aRetval.push_back(DomMapFlag::OOXDrawing);
    // aRetval.push_back(DomMapFlag::OOXDataRels);
    // aRetval.push_back(DomMapFlag::OOXLayout);

    return aRetval;
}

DiagramDataState::DiagramDataState(Connections aConnections, Points aPoints)
: maConnections(std::move(aConnections))
, maPoints(std::move(aPoints))
{
}

DiagramDataStatePtr DiagramData_svx::extractDiagramDataState() const
{
    // Just copy all Connections && Points. The shared_ptr data in
    // Point-entries is no problem, it just continues exiting shared
    return std::make_shared< DiagramDataState >(maConnections, maPoints);
}

void DiagramData_svx::applyDiagramDataState(const DiagramDataStatePtr& rState)
{
    if(rState)
    {
        maConnections = rState->getConnections();
        maPoints = rState->getPoints();

        // Reset temporary buffered ModelData association lists & rebuild them
        // and the Diagram DataModel. Do that here *immediately* to prevent
        // re-usage of potentially invalid Connection/Point objects
        buildDiagramDataModel(true);
    }
}

void DiagramData_svx::getDiagramChildrenString(
    OUStringBuffer& rBuf,
    const svx::diagram::Point* pPoint,
    sal_Int32 nLevel,
    const css::uno::Reference<css::drawing::XShape>& rRootShape) const
{
    if (!pPoint)
        return;

    if (nLevel > 0)
    {
        for (sal_Int32 i = 0; i < nLevel-1; i++)
            rBuf.append('\t');
        rBuf.append('+');
        rBuf.append(' ');
        const OUString aText(getTextForPoint(*pPoint, rRootShape));
        rBuf.append(aText);
        rBuf.append('\n');
    }

    std::vector< const svx::diagram::Point* > aChildren;
    for (const auto& rCxn : maConnections)
        if (rCxn.mnXMLType == TypeConstant::XML_parOf && rCxn.msSourceId == pPoint->msModelId)
        {
            if (rCxn.mnSourceOrder >= static_cast<sal_Int32>(aChildren.size()))
                aChildren.resize(rCxn.mnSourceOrder + 1);
            const auto pChild = maPointNameMap.find(rCxn.msDestId);
            if (pChild != maPointNameMap.end())
                aChildren[rCxn.mnSourceOrder] = pChild->second;
        }

    for (auto pChild : aChildren)
        getDiagramChildrenString(rBuf, pChild, nLevel + 1, rRootShape);
}

uno::Reference<drawing::XShape> DiagramData_svx::getXShapeByModelID(const uno::Reference<drawing::XShape>& rxShape, std::u16string_view rModelID)
{
    if (!rxShape)
        return rxShape;

    uno::Reference<drawing::XShape> xRetval;
    if (rModelID.empty())
        return xRetval;

    SdrObject* pCandidate(SdrObject::getSdrObjectFromXShape(rxShape));
    if (nullptr == pCandidate)
        return xRetval;

    SdrObjListIter aIterator(*pCandidate, SdrIterMode::DeepNoGroups);
    while (aIterator.IsMore())
    {
        pCandidate = aIterator.Next();
        if (nullptr != pCandidate && rModelID == pCandidate->getDiagramDataModelID())
            return pCandidate->getUnoShape();
    }

    return xRetval;
}

uno::Reference<drawing::XShape> DiagramData_svx::getMasterXShapeForPoint(const Point& rPoint, const uno::Reference<drawing::XShape>& rRootShape) const
{
    if (!rPoint.msPlaceholderText.isEmpty())
    {
        for (auto& rCandidate : getPoints())
        {
            if (!rCandidate.msPresentationAssociationId.isEmpty() && rCandidate.msPresentationAssociationId == rPoint.msModelId)
            {
                uno::Reference<drawing::XShape> xMasterText = getXShapeByModelID(rRootShape, rCandidate.msModelId);
                if (xMasterText)
                    return xMasterText;
            }
        }
    }

    return uno::Reference<drawing::XShape>();
}

OUString DiagramData_svx::getTextForPoint(const Point& rPoint, const uno::Reference<drawing::XShape>& rRootShape) const
{
    uno::Reference<drawing::XShape> xMasterText(getMasterXShapeForPoint(rPoint, rRootShape));
    uno::Reference<text::XText> xText(xMasterText, uno::UNO_QUERY);

    if (xText)
        return xText->getString();

    return OUString();
}

std::vector<std::pair<OUString, OUString>> DiagramData_svx::getDiagramChildren(const OUString& rParentId, const uno::Reference<drawing::XShape>& rRootShape) const
{
    const OUString sModelId = rParentId.isEmpty() ? getRootPoint()->msModelId : rParentId;
    std::vector<std::pair<OUString, OUString>> aChildren;
    for (const auto& rCxn : maConnections)
        if (rCxn.mnXMLType == TypeConstant::XML_parOf && rCxn.msSourceId == sModelId)
        {
            if (rCxn.mnSourceOrder >= static_cast<sal_Int32>(aChildren.size()))
                aChildren.resize(rCxn.mnSourceOrder + 1);
            const auto pChild = maPointNameMap.find(rCxn.msDestId);
            if (pChild != maPointNameMap.end())
            {
                const OUString aText(getTextForPoint(*pChild->second, rRootShape));
                aChildren[rCxn.mnSourceOrder] = std::make_pair(
                    pChild->second->msModelId,
                    aText);
            }
        }

    // HACK: empty items shouldn't appear there
    std::erase_if(aChildren, [](const std::pair<OUString, OUString>& aItem) { return aItem.first.isEmpty(); });

    return aChildren;
}

std::pair<OUString, DomMapFlags> DiagramData_svx::addDiagramNode()
{
    DomMapFlags aRetval;
    const svx::diagram::Point& rDataRoot = *getRootPoint();
    OUString sPresRoot;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_presOf && aCxn.msSourceId == rDataRoot.msModelId)
            sPresRoot = aCxn.msDestId;

    if (sPresRoot.isEmpty())
        return std::make_pair(OUString(), aRetval);

    OUString sNewNodeId = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);

    svx::diagram::Point aDataPoint;
    aDataPoint.mnXMLType = TypeConstant::XML_node;
    aDataPoint.msModelId = sNewNodeId;
    aDataPoint.msPlaceholderText = "[Text]";

    OUString sDataSibling;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_parOf && aCxn.msSourceId == rDataRoot.msModelId)
            sDataSibling = aCxn.msDestId;

    OUString sPresSibling;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_presOf && aCxn.msSourceId == sDataSibling)
            sPresSibling = aCxn.msDestId;

    svx::diagram::Point aPresPoint;
    aPresPoint.mnXMLType = TypeConstant::XML_pres;
    aPresPoint.msModelId = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);

    aPresPoint.msPresentationAssociationId = aDataPoint.msModelId;
    if (!sPresSibling.isEmpty())
    {
        // no idea where to get these values from, so copy from previous sibling
        const svx::diagram::Point* pSiblingPoint = maPointNameMap[sPresSibling];
        aPresPoint.msPresentationLayoutName = pSiblingPoint->msPresentationLayoutName;
        aPresPoint.msPresentationLayoutStyleLabel = pSiblingPoint->msPresentationLayoutStyleLabel;
        aPresPoint.mnLayoutStyleIndex = pSiblingPoint->mnLayoutStyleIndex;
        aPresPoint.mnLayoutStyleCount = pSiblingPoint->mnLayoutStyleCount;
    }

    addConnection(svx::diagram::TypeConstant::XML_parOf, rDataRoot.msModelId, aDataPoint.msModelId);
    addConnection(svx::diagram::TypeConstant::XML_presParOf, sPresRoot, aPresPoint.msModelId);
    addConnection(svx::diagram::TypeConstant::XML_presOf, aDataPoint.msModelId, aPresPoint.msModelId);

    // adding at the end, so that references are not invalidated in between
    maPoints.push_back(std::move(aDataPoint));
    maPoints.push_back(std::move(aPresPoint));

    // prepare retval, OOXData and OOXLayout is changed
    aRetval.push_back(DomMapFlag::OOXData);
    // aRetval.push_back(DomMapFlag::OOXDrawing);
    // aRetval.push_back(DomMapFlag::OOXDataRels);
    // aRetval.push_back(DomMapFlag::OOXLayout);

    return std::make_pair(sNewNodeId, aRetval);
}

void DiagramData_svx::addConnection(svx::diagram::TypeConstant nType, const OUString& sSourceId, const OUString& sDestId)
{
    sal_Int32 nMaxOrd = -1;
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == nType && aCxn.msSourceId == sSourceId)
            nMaxOrd = std::max(nMaxOrd, aCxn.mnSourceOrder);

    svx::diagram::Connection& rCxn = maConnections.emplace_back();
    rCxn.mnXMLType = nType;
    rCxn.msSourceId = sSourceId;
    rCxn.msDestId = sDestId;
    rCxn.mnSourceOrder = nMaxOrd + 1;
}

// #define DEBUG_OOX_DIAGRAM
#ifdef DEBUG_OOX_DIAGRAM
OString normalizeDotName( const OUString& rStr )
{
    OUStringBuffer aBuf;
    aBuf.append('N');

    const sal_Int32 nLen(rStr.getLength());
    sal_Int32 nCurrIndex(0);
    while( nCurrIndex < nLen )
    {
        const sal_Int32 aChar=rStr.iterateCodePoints(&nCurrIndex);
        if( aChar != '-' && aChar != '{' && aChar != '}' )
            aBuf.append((sal_Unicode)aChar);
    }

    return OUStringToOString(aBuf.makeStringAndClear(),
                                  RTL_TEXTENCODING_UTF8);
}
#endif

static sal_Int32 calcDepth( std::u16string_view rNodeName,
                            const svx::diagram::Connections& rCnx )
{
    // find length of longest path in 'isChild' graph, ending with rNodeName
    for (auto const& elem : rCnx)
    {
        if( !elem.msParTransId.isEmpty() &&
            !elem.msSibTransId.isEmpty() &&
            !elem.msSourceId.isEmpty() &&
            !elem.msDestId.isEmpty() &&
            elem.mnXMLType == TypeConstant::XML_parOf &&
            rNodeName == elem.msDestId )
        {
            return calcDepth(elem.msSourceId, rCnx) + 1;
        }
    }

    return 0;
}

void DiagramData_svx::buildDiagramDataModel(bool /*bClearOoxShapes*/)
{
    // build name-object maps
    maPointNameMap.clear();
    maPointsPresNameMap.clear();
    maConnectionNameMap.clear();
    maPresOfNameMap.clear();

#ifdef DEBUG_OOX_DIAGRAM
    std::ofstream output("tree.dot");

    output << "digraph datatree {" << std::endl;
#endif
    svx::diagram::Points& rPoints = getPoints();
    for (auto & point : rPoints)
    {
#ifdef DEBUG_OOX_DIAGRAM
        output << "\t"
               << normalizeDotName(point.msModelId).getStr()
               << "[";

        if( !point.msPresentationLayoutName.isEmpty() )
            output << "label=\""
                   << OUStringToOString(
                       point.msPresentationLayoutName,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";
        else
            output << "label=\""
                   << OUStringToOString(
                       point.msModelId,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";

        switch( point.mnXMLType )
        {
            case TypeConstant::XML_doc: output << "style=filled, color=red"; break;
            case TypeConstant::XML_asst: output << "style=filled, color=green"; break;
            default:
            case TypeConstant::XML_node: output << "style=filled, color=blue"; break;
            case TypeConstant::XML_pres: output << "style=filled, color=yellow"; break;
            case TypeConstant::XML_parTrans: output << "color=grey"; break;
            case TypeConstant::XML_sibTrans: output << " "; break;
        }

        output << "];" << std::endl;
#endif

        const bool bInserted1 = getPointNameMap().insert(
            std::make_pair(point.msModelId,&point)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "DiagramData_svx::build(): non-unique point model id");

        if( !point.msPresentationLayoutName.isEmpty() )
        {
            DiagramData_svx::PointsNameMap::value_type::second_type& rVec=
                getPointsPresNameMap()[point.msPresentationLayoutName];
            rVec.push_back(&point);
        }
    }

    const svx::diagram::Connections& rConnections = getConnections();
    for (auto const& connection : rConnections)
    {
#ifdef DEBUG_OOX_DIAGRAM
        if( !connection.msParTransId.isEmpty() ||
            !connection.msSibTransId.isEmpty() )
        {
            if( !connection.msSourceId.isEmpty() ||
                !connection.msDestId.isEmpty() )
            {
                output << "\t"
                       << normalizeDotName(connection.msSourceId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msSibTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msDestId).getStr()
                       << " [style=dotted,"
                       << ((connection.mnXMLType == TypeConstant::XML_presOf) ? " color=red, " : ((connection.mnXMLType == TypeConstant::XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(connection.msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
            else
            {
                output << "\t"
                       << normalizeDotName(connection.msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection.msSibTransId).getStr()
                       << " ["
                       << ((connection.mnXMLType == TypeConstant::XML_presOf) ? " color=red, " : ((connection.mnXMLType == TypeConstant::XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(connection.msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
        }
        else if( !connection.msSourceId.isEmpty() ||
                 !connection.msDestId.isEmpty() )
            output << "\t"
                   << normalizeDotName(connection.msSourceId).getStr()
                   << " -> "
                   << normalizeDotName(connection.msDestId).getStr()
                   << " [label=\""
                   << OUStringToOString(connection.msModelId,
                                             RTL_TEXTENCODING_UTF8 ).getStr()
                   << ((connection.mnXMLType == TypeConstant::XML_presOf) ? "\", color=red]" : ((connection.mnXMLType == TypeConstant::XML_presParOf) ? "\", color=green]" : "\"]"))
                   << ";" << std::endl;
#endif

        const bool bInserted1 = maConnectionNameMap.insert(
            std::make_pair(connection.msModelId,&connection)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "DiagramData_svx::build(): non-unique connection model id");

        if( connection.mnXMLType == TypeConstant::XML_presOf )
        {
            DiagramData_svx::StringMap::value_type::second_type& rVec = getPresOfNameMap()[connection.msDestId];
            rVec[connection.mnDestOrder] = { connection.msSourceId, sal_Int32(0) };
        }
    }

    // assign outline levels
    DiagramData_svx::StringMap& rStringMap = getPresOfNameMap();
    for (auto & elemPresOf : rStringMap)
    {
        for (auto & elem : elemPresOf.second)
        {
            const sal_Int32 nDepth = calcDepth(elem.second.msSourceId, getConnections());
            elem.second.mnDepth = nDepth != 0 ? nDepth : -1;
        }
    }
#ifdef DEBUG_OOX_DIAGRAM
    output << "}" << std::endl;
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
