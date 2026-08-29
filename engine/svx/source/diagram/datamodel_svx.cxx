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

#include <cassert>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <fstream>

#include <svx/diagram/datamodel_svx.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <comphelper/xmltools.hxx>
#include <sal/log.hxx>
#include <utility>
#include <sax/fastattribs.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

using namespace ::oox;
using namespace ::com::sun::star;

namespace svx::diagram {

std::u16string_view getNameForTypeConstant(TypeConstant aTypeConstant)
{
    switch (aTypeConstant)
    {
        case TypeConstant::XML_type: return u"Type"; break;
        case TypeConstant::XML_asst: return u"asst"; break;
        case TypeConstant::XML_doc: return u"doc"; break;
        case TypeConstant::XML_node: return u"node"; break;
        case TypeConstant::XML_norm: return u"norm"; break;
        case TypeConstant::XML_parOf: return u"parOf"; break;
        case TypeConstant::XML_parTrans: return u"parTrans"; break;
        case TypeConstant::XML_pres: return u"pres"; break;
        case TypeConstant::XML_presOf: return u"presOf"; break;
        case TypeConstant::XML_presParOf: return u"presParOf"; break;
        case TypeConstant::XML_rel: return u"rel"; break;
        case TypeConstant::XML_sibTrans: return u"sibTrans"; break;
        case TypeConstant::XML_none: break;
    }

    return u"";
}

static OUString getNameForOOXToken(sal_Int32 nToken)
{
    // values for XML_hierBranch
    if (oox::XML_init == nToken)
        return u"init"_ustr;
    else if (oox::XML_hang == nToken)
        return u"hang"_ustr;
    else if (oox::XML_l == nToken)
        return u"l"_ustr;
    else if (oox::XML_r == nToken)
        return u"r"_ustr;
    else if (oox::XML_std == nToken)
        return u"std"_ustr;
    // values for XML_dir
    else if (oox::XML_rev == nToken)
        return u"rev"_ustr;
    else if (oox::XML_norm == nToken)
        return u"norm"_ustr;
    else
        return u""_ustr;
}

void addTypeConstantToFastAttributeList(TypeConstant aTypeConstant, rtl::Reference<sax_fastparser::FastAttributeList>& rAttributeList, bool bPoint)
{
    if (TypeConstant::XML_none == aTypeConstant)
        return;
    if (TypeConstant::XML_node == aTypeConstant && bPoint)
        return;
    if (TypeConstant::XML_parOf == aTypeConstant && !bPoint)
        return;

    const std::u16string_view aName(getNameForTypeConstant(aTypeConstant));
    // *CAUTION!* here '::XML_type' is *not* the same as 'XML_type' which would
    // namespace expand to oox::XML_type as in enum TypeConstant definitions (!)
    if (!aName.empty())
        rAttributeList->add(::XML_type, aName);
}

Points copyPoints(const Points& rSource)
{
    Points aRetval;
    aRetval.reserve(rSource.size());

    for (const rtl::Reference<Point>& rPoint : rSource)
        aRetval.push_back(rPoint.is() ? new Point(*rPoint) : nullptr);

    return aRetval;
}

Connections copyConnections(const Connections& rSource)
{
    Connections aRetval;
    aRetval.reserve(rSource.size());

    for (const rtl::Reference<Connection>& rConnection : rSource)
        aRetval.push_back(rConnection.is() ? new Connection(*rConnection) : nullptr);

    return aRetval;
}

Connection::Connection()
{
}

Connection::Connection(const Connection& rSource)
: salhelper::SimpleReferenceObject()
, ConnectionValues(rSource)
{
}

Connection::~Connection()
{
}

void Connection::writeDiagramData_connection(sax_fastparser::FSHelperPtr& rTarget)
{
    if (!rTarget)
        return;

    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(sax_fastparser::FastSerializerHelper::createAttrList());

    if (!msModelId.isEmpty())
        pAttributeList->add(XML_modelId, msModelId);
    addTypeConstantToFastAttributeList(mnXMLType, pAttributeList, false);
    if (!msSourceId.isEmpty())
        pAttributeList->add(XML_srcId, msSourceId);
    if (!msDestId.isEmpty())
        pAttributeList->add(XML_destId, msDestId);
    pAttributeList->add(XML_srcOrd, OUString::number(mnSourceOrder));
    pAttributeList->add(XML_destOrd, OUString::number(mnDestOrder));
    if (!msPresId.isEmpty())
        pAttributeList->add(XML_presId, msPresId);
    if (!msParTransId.isEmpty())
        pAttributeList->add(XML_parTransId, msParTransId);
    if (!msSibTransId.isEmpty())
        pAttributeList->add(XML_sibTransId, msSibTransId);

    rTarget->singleElementNS(XML_dgm, XML_cxn, pAttributeList);
}

Point::Point()
{
}

Point::Point(const Point& rSource)
: salhelper::SimpleReferenceObject()
, PointValues(rSource)
{
}

Point::~Point()
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
    if (mbCoherent3DOffset) pAttributeList->add(XML_coherent3DOff, aStrTrue);
    if (mbCustomHorizontalFlip) pAttributeList->add(XML_custFlipHor, aStrTrue);
    if (mbCustomVerticalFlip) pAttributeList->add(XML_custFlipVert, aStrTrue);
    if (mbCustomText) pAttributeList->add(XML_custT, aStrTrue);
    if (mbIsPlaceholder) pAttributeList->add(XML_phldr, aStrTrue);

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

        if (mbOrgChartEnabled)
            rTarget->singleElementNS(XML_dgm, XML_orgChart, XML_val, aStrTrue);

        if (-1 != mnMaxChildren)
            rTarget->singleElementNS(XML_dgm, XML_chMax, XML_val, OUString::number(mnMaxChildren));

        if (-1 != mnPreferredChildren)
            rTarget->singleElementNS(XML_dgm, XML_chPref, XML_val, OUString::number(mnPreferredChildren));

        if (mbBulletEnabled)
            rTarget->singleElementNS(XML_dgm, XML_bulletEnabled, XML_val, aStrTrue);

        if (XML_norm != mnDirection)
            rTarget->singleElementNS(XML_dgm, XML_dir, XML_val, getNameForOOXToken(mnDirection));

        if (moHierarchyBranch.has_value())
            rTarget->singleElementNS(XML_dgm, XML_hierBranch, XML_val, getNameForOOXToken(moHierarchyBranch.value()));

        // ToDo: animOne not implemented

        // ToDo: animLvl not implemented

        if (!msResizeHandles.isEmpty())
            rTarget->singleElementNS(XML_dgm, XML_resizeHandles, XML_val, msResizeHandles);

        rTarget->endElementNS(XML_dgm, XML_presLayoutVars);
        rTarget->endElementNS(XML_dgm, XML_prSet);
    }
    else
        rTarget->singleElementNS(XML_dgm, XML_prSet, pAttributeList);
}

DiagramData_svx::DiagramData_svx()
: mxRootShape()
, maExtDrawings()
, maConnections()
, maPoints()
, mxThemeDocument()
, maPointsPresNameMap()
, maConnectionNameMap()
, maPresOfNameMap()
{
}

DiagramData_svx::DiagramData_svx(DiagramData_svx const& rSource)
: mxRootShape()
, maExtDrawings()
, maConnections(copyConnections(rSource.maConnections))
, maPoints(copyPoints(rSource.maPoints))
, mxThemeDocument()
, maPointsPresNameMap()
, maConnectionNameMap()
, maPresOfNameMap()
{
}

const uno::Reference< frame::XModel >& DiagramData_svx::accessRootModel() const
{
    SdrObject* pObj(SdrObject::getSdrObjectFromXShape(accessRootShape()));
    SAL_WARN_IF(!pObj, "oox.drawingml", "DiagramData_svx::accessRootModel(): no XModel");
    return pObj->getSdrModelFromSdrObject().getUnoModel();
}

DiagramData_svx::~DiagramData_svx()
{
}

rtl::Reference<Point> DiagramData_svx::getRootPoint() const
{
    for (const rtl::Reference<Point>& rCurrentPoint : maPoints)
        if (rCurrentPoint->mnXMLType == TypeConstant::XML_doc)
            return rCurrentPoint;

    SAL_WARN("svx.diagram", "No root point");
    return nullptr;
}

OUString DiagramData_svx::getDiagramString() const
{
    OUStringBuffer aBuf;
    getDiagramChildrenString(aBuf, getRootPoint(), 0);
    return aBuf.makeStringAndClear();
}

namespace
{
// Creates a fresh ModelId
OUString createModelId()
{
    return OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);
}

// Adds a Connection that carries all of the given values
Connection& appendConnection(Connections& rConnections, TypeConstant nType, const OUString& rModelId,
                            const OUString& rSourceId, const OUString& rDestId,
                            sal_Int32 nSourceOrder, const OUString& rLayoutId)
{
    Connection& rCxn(*rConnections.emplace_back(new Connection));

    rCxn.mnXMLType = nType;
    rCxn.msModelId = rModelId;
    rCxn.msSourceId = rSourceId;
    rCxn.msDestId = rDestId;
    rCxn.mnSourceOrder = nSourceOrder;
    rCxn.msPresId = rLayoutId;

    return rCxn;
}

// Closes the gaps that removing entries leaves in the order values of the surviving Connections
void correctConnectionOrders(Connections& rConnections)
{
    std::map<std::pair<TypeConstant, OUString>, std::vector<Connection*>> aGroupedBySource;
    std::map<std::pair<TypeConstant, OUString>, std::vector<Connection*>> aGroupedByDestination;

    for (const rtl::Reference<Connection>& rCxn : rConnections)
    {
        aGroupedBySource[{ rCxn->mnXMLType, rCxn->msSourceId }].push_back(rCxn.get());
        aGroupedByDestination[{ rCxn->mnXMLType, rCxn->msDestId }].push_back(rCxn.get());
    }

    for (auto& rGroup : aGroupedBySource)
    {
        std::stable_sort(rGroup.second.begin(), rGroup.second.end(),
            [](const Connection* pA, const Connection* pB)
            { return pA->mnSourceOrder < pB->mnSourceOrder; });

        sal_Int32 nOrder(0);
        for (Connection* pCxn : rGroup.second)
            pCxn->mnSourceOrder = nOrder++;
    }

    for (auto& rGroup : aGroupedByDestination)
    {
        std::stable_sort(rGroup.second.begin(), rGroup.second.end(),
            [](const Connection* pA, const Connection* pB)
            { return pA->mnDestOrder < pB->mnDestOrder; });

        sal_Int32 nOrder(0);
        for (Connection* pCxn : rGroup.second)
            pCxn->mnDestOrder = nOrder++;
    }
}

// Closes the gaps that removing entries leaves in the presentation style indexes of the surviving
// Points. The indexes run per style label, and two Points that hold the same index belong to the
// same item of the Diagram and go on holding one index between them, as a Point that represents
//  the text of a node and a Point that represents its connector do. So the distinct index values
//  are renumbered rather than the Points, and the count becomes the number of distinct values.
void correctPresentationStyleIndexes(Points& rPoints)
{
    std::map<OUString, std::vector<Point*>> aGroupedByLabel;

    for (const rtl::Reference<Point>& rPoint : rPoints)
        if (!rPoint->msPresentationLayoutStyleLabel.isEmpty() && rPoint->mnLayoutStyleIndex >= 0)
            aGroupedByLabel[rPoint->msPresentationLayoutStyleLabel].push_back(rPoint.get());

    for (auto& rGroup : aGroupedByLabel)
    {
        // the index values the label holds, in ascending order, each mapped to its new value
        std::map<sal_Int32, sal_Int32> aNewIndexForOld;

        for (const Point* xPoint : rGroup.second)
            aNewIndexForOld[xPoint->mnLayoutStyleIndex] = 0;

        sal_Int32 nNextIndex(0);

        for (auto& rEntry : aNewIndexForOld)
            rEntry.second = nNextIndex++;

        const sal_Int32 nCount(static_cast<sal_Int32>(aNewIndexForOld.size()));

        for (Point* xPoint : rGroup.second)
        {
            xPoint->mnLayoutStyleIndex = aNewIndexForOld[xPoint->mnLayoutStyleIndex];
            xPoint->mnLayoutStyleCount = nCount;
        }
    }
}

// The presentation Points in the order the Diagram represents them, each one mapped to its place in
// that order: a container comes first and what hangs inside it follows, each of those in the order
// it hangs there. The presParOf Connections give that order. A Point that the walk does not reach
// gets no entry.
std::map<OUString, size_t> readDrawingOrderOfPresentationPoints(const Points& rPoints,
                                                                const Connections& rConnections)
{
    // what hangs inside each presentation Point, in the order it hangs there
    std::map<OUString, std::vector<const Connection*>> aContentsOfContainer;
    std::unordered_set<OUString> aHangsSomewhere;

    for (const rtl::Reference<Connection>& rCxn : rConnections)
        if (TypeConstant::XML_presParOf == rCxn->mnXMLType)
        {
            aContentsOfContainer[rCxn->msSourceId].push_back(rCxn.get());
            aHangsSomewhere.insert(rCxn->msDestId);
        }

    for (auto& rEntry : aContentsOfContainer)
        std::stable_sort(rEntry.second.begin(), rEntry.second.end(),
                         [](const Connection* pA, const Connection* pB)
                         { return pA->mnSourceOrder < pB->mnSourceOrder; });

    // The walk starts at the presentation Points that hang under nothing, of which a Diagram has
    // the one that the layout hangs everything else under. They go on in reverse, the walk takes
    // them off the back.
    std::vector<OUString> aPending;

    for (auto aIter(rPoints.rbegin()); aIter != rPoints.rend(); ++aIter)
        if (TypeConstant::XML_pres == (*aIter)->mnXMLType
            && !aHangsSomewhere.count((*aIter)->msModelId))
            aPending.push_back((*aIter)->msModelId);

    std::map<OUString, size_t> aRetval;

    while (!aPending.empty())
    {
        const OUString aCurrentId(aPending.back());
        aPending.pop_back();

        if (aRetval.count(aCurrentId))
            continue;

        const size_t nPlace(aRetval.size());
        aRetval[aCurrentId] = nPlace;

        const auto aFound(aContentsOfContainer.find(aCurrentId));

        if (aFound == aContentsOfContainer.end())
            continue;

        for (auto aIter(aFound->second.rbegin()); aIter != aFound->second.rend(); ++aIter)
            aPending.push_back((*aIter)->msDestId);
    }

    return aRetval;
}

// Where a presentation Point comes in the order the Diagram represents them. One that the order does
// not name comes behind all of them.
size_t readDrawingPlace(const std::map<OUString, size_t>& rOrder, const OUString& rModelId)
{
    const auto aFound(rOrder.find(rModelId));

    return aFound == rOrder.end() ? rOrder.size() : aFound->second;
}

// Puts the presentation Points in the order the Diagram represents them. The data Points keep their
// places, only the presentation Points move, and each of them lands in a place that a presentation
// Point holds now.
void sortPresentationPointsIntoDrawingOrder(Points& rPoints, const Connections& rConnections)
{
    const std::map<OUString, size_t> aDrawingOrder(
        readDrawingOrderOfPresentationPoints(rPoints, rConnections));

    // the places that presentation Points hold now, and the Points that sit in them
    std::vector<size_t> aPlaces;
    Points aInDrawingOrder;

    for (size_t a(0); a < rPoints.size(); a++)
        if (TypeConstant::XML_pres == rPoints[a]->mnXMLType)
        {
            aPlaces.push_back(a);
            aInDrawingOrder.push_back(rPoints[a]);
        }

    std::stable_sort(aInDrawingOrder.begin(), aInDrawingOrder.end(),
                     [&aDrawingOrder](const rtl::Reference<Point>& rA,
                                      const rtl::Reference<Point>& rB)
                     {
                         return readDrawingPlace(aDrawingOrder, rA->msModelId)
                                < readDrawingPlace(aDrawingOrder, rB->msModelId);
                     });

    // back into those places, now in that order
    for (size_t a(0); a < aPlaces.size(); a++)
        rPoints[aPlaces[a]] = aInDrawingOrder[a];
}
}



bool DiagramData_svx::canHoldChildNode(std::u16string_view rNodeId) const
{
    rtl::Reference<Point> xTarget(getDataNodeForModelID(rNodeId));

    if (!xTarget.is())
        return false;

    const OUString aNodeId(xTarget->msModelId);

    // a node that already holds nodes below it plainly can hold one more
    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msSourceId == aNodeId)
            return true;

    // Otherwise the layout has to draw a place to put them. Collect the names of the containers
    // that nodes below a node hang in anywhere in the Diagram, then look for one of each name
    // under this node. A layout that represents no node below a node names none, and then no
    // node can take one.
    std::set<OUString> aContainerNames;

    for (const rtl::Reference<Connection>& rCxn : maConnections)
    {
        if (TypeConstant::XML_parOf != rCxn->mnXMLType)
            continue;

        rtl::Reference<Point> xParent(getPointByModelID(rCxn->msSourceId));

        if (!xParent.is() || TypeConstant::XML_doc == xParent->mnXMLType)
            continue;

        for (const rtl::Reference<Point>& rPoint : maPoints)
        {
            if (TypeConstant::XML_pres != rPoint->mnXMLType
                || rPoint->msPresentationAssociationId != rCxn->msDestId)
                continue;

            for (const rtl::Reference<Connection>& rOther : maConnections)
                if (TypeConstant::XML_presParOf == rOther->mnXMLType
                    && rOther->msDestId == rPoint->msModelId)
                {
                    rtl::Reference<Point> xContainer(getPointByModelID(rOther->msSourceId));

                    if (xContainer.is() && !xContainer->msPresentationLayoutName.isEmpty())
                        aContainerNames.insert(xContainer->msPresentationLayoutName);
                }
        }
    }

    if (aContainerNames.empty())
        return false;

    for (const OUString& rContainerName : aContainerNames)
    {
        bool bFound(false);

        for (const rtl::Reference<Point>& rPoint : maPoints)
            if (TypeConstant::XML_pres == rPoint->mnXMLType
                && rPoint->msPresentationAssociationId == aNodeId
                && rPoint->msPresentationLayoutName == rContainerName)
                bFound = true;

        if (!bFound)
            return false;
    }

    return true;
}

bool DiagramData_svx::isChildNode(std::u16string_view rNodeId) const
{
    rtl::Reference<Point> xTarget(getDataNodeForModelID(rNodeId));

    if (!xTarget.is())
        return false;

    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msSourceId == xTarget->msModelId)
            return false;

    return true;
}

bool DiagramData_svx::isPresentationOfDataNode(std::u16string_view rModelId) const
{
    rtl::Reference<Point> xPoint(getPointByModelID(rModelId));

    if (!xPoint.is() || TypeConstant::XML_pres != xPoint->mnXMLType
        || xPoint->msPresentationAssociationId.isEmpty())
        return false;

    rtl::Reference<Point> xAssociated(getPointByModelID(xPoint->msPresentationAssociationId));

    // The Diagram as a whole is a Point of its own, of the document type, and so is each of the
    // two transition Points that a Connection brings along. Only a Point of the node type is one
    // of the nodes that the Diagram holds.
    return xAssociated.is() && TypeConstant::XML_node == xAssociated->mnXMLType;
}

rtl::Reference<Point> DiagramData_svx::getDataNodeForModelID(std::u16string_view rModelId) const
{
    rtl::Reference<Point> xRetval(getPointByModelID(rModelId));

    // a presentation Point names the data node that it was made for
    if (xRetval.is() && TypeConstant::XML_pres == xRetval->mnXMLType)
        xRetval = xRetval->msPresentationAssociationId.isEmpty()
            ? nullptr
            : getPointByModelID(xRetval->msPresentationAssociationId);

    // a transition Point reaches the data node through the Connection that it belongs to
    if (xRetval.is()
        && (TypeConstant::XML_parTrans == xRetval->mnXMLType
            || TypeConstant::XML_sibTrans == xRetval->mnXMLType))
    {
        rtl::Reference<Connection> xOwner;

        if (!xRetval->msCnxId.isEmpty())
            for (const rtl::Reference<Connection>& rCxn : maConnections)
                if (rCxn->msModelId == xRetval->msCnxId)
                {
                    xOwner = rCxn;
                    break;
                }

        xRetval = xOwner.is() ? getPointByModelID(xOwner->msDestId) : nullptr;
    }

    return xRetval;
}

DomMapFlags DiagramData_svx::removeDiagramNode(std::u16string_view rNodeId, bool bRemoveAllChildren)
{
    DomMapFlags aRetval;

    // The incoming ModelId comes from a shape, so it usually names the presentation Point that the
    // shape was created for, and not the data node behind it.
    rtl::Reference<Point> xTarget(getDataNodeForModelID(rNodeId));

    if (!xTarget.is())
    {
        SAL_WARN("svx.diagram", "removeDiagramNode: the given ModelId leads to no data node");
        return aRetval;
    }

    if (TypeConstant::XML_doc == xTarget->mnXMLType)
    {
        SAL_WARN("svx.diagram", "removeDiagramNode: the root node cannot be removed");
        return aRetval;
    }

    // Copy the ModelId. The Point itself is about to be erased from maPoints.
    const OUString aDataNodeId(xTarget->msModelId);

    // A node that still holds nodes below it stays, unless the whole branch was asked for.
    if (!bRemoveAllChildren)
        for (const rtl::Reference<Connection>& rCxn : maConnections)
            if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msSourceId == aDataNodeId)
            {
                SAL_WARN("svx.diagram", "removeDiagramNode: the node holds nodes below it and stays");
                return aRetval;
            }

    // Collect the data Points to remove: the node itself and its two transition Points. A
    // transition Point is named only by the parTransId and sibTransId of the parOf Connection
    // that leads to the node
    std::unordered_set<OUString> aDeadPoints;
    std::unordered_set<OUString> aDeadConnections;

    // The parent of the removed node and the place it held among its siblings.
    OUString aParentId;
    sal_Int32 nSourceOrder(0);
    std::vector<OUString> aPending{ aDataNodeId };

    while (!aPending.empty())
    {
        const OUString aCurrentId(aPending.back());
        aPending.pop_back();

        if (!aDeadPoints.insert(aCurrentId).second)
            continue;

        for (const rtl::Reference<Connection>& rCxn : maConnections)
        {
            if (TypeConstant::XML_parOf != rCxn->mnXMLType)
                continue;

            if (rCxn->msDestId == aCurrentId)
            {
                aDeadConnections.insert(rCxn->msModelId);

                if (!rCxn->msParTransId.isEmpty())
                    aDeadPoints.insert(rCxn->msParTransId);

                if (!rCxn->msSibTransId.isEmpty())
                    aDeadPoints.insert(rCxn->msSibTransId);

                if (aCurrentId == aDataNodeId)
                {
                    aParentId = rCxn->msSourceId;
                    nSourceOrder = rCxn->mnSourceOrder;
                }
            }
            else if (bRemoveAllChildren && rCxn->msSourceId == aCurrentId)
            {
                aPending.push_back(rCxn->msDestId);
            }
        }
    }

    // The Diagram always keeps one node, so a removal that would leave it empty does not happen.
    bool bNodeSurvives(false);

    for (const rtl::Reference<Point>& rPoint : maPoints)
        if (TypeConstant::XML_node == rPoint->mnXMLType && !aDeadPoints.count(rPoint->msModelId))
        {
            bNodeSurvives = true;
            break;
        }

    if (!bNodeSurvives)
    {
        SAL_WARN("svx.diagram", "removeDiagramNode: the Diagram keeps its last node");
        return aRetval;
    }

    // The removed node was the last child of its parent, so the separator that used to sit
    // between it and the node before it now trails the diagram with nothing behind it
    if (!aParentId.isEmpty())
    {
        const Connection* pPreviousSibling(nullptr);
        bool bWasLastChild(true);

        for (const rtl::Reference<Connection>& rCxn : maConnections)
        {
            if (TypeConstant::XML_parOf != rCxn->mnXMLType || rCxn->msSourceId != aParentId
                || aDeadConnections.count(rCxn->msModelId))
                continue;

            if (rCxn->mnSourceOrder > nSourceOrder)
                bWasLastChild = false;
            else if (nullptr == pPreviousSibling
                     || rCxn->mnSourceOrder > pPreviousSibling->mnSourceOrder)
                pPreviousSibling = rCxn.get();
        }

        if (bWasLastChild && nullptr != pPreviousSibling
            && !pPreviousSibling->msSibTransId.isEmpty())
        {
            for (const rtl::Reference<Point>& rPoint : maPoints)
                if (rPoint->msPresentationAssociationId == pPreviousSibling->msSibTransId)
                    aDeadPoints.insert(rPoint->msModelId);
        }
    }

    // Every Point that presents one of the dead Points is dead too
    bool bSetGrew(true);

    while (bSetGrew)
    {
        bSetGrew = false;

        for (const rtl::Reference<Point>& rPoint : maPoints)
            if (!rPoint->msPresentationAssociationId.isEmpty()
                && aDeadPoints.count(rPoint->msPresentationAssociationId)
                && aDeadPoints.insert(rPoint->msModelId).second)
                bSetGrew = true;
    }

    // A Connection with a dead Point at either end goes with it.
    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (aDeadPoints.count(rCxn->msSourceId) || aDeadPoints.count(rCxn->msDestId))
            aDeadConnections.insert(rCxn->msModelId);

    // erase dead connections
    std::erase_if(maConnections, [&aDeadConnections](const rtl::Reference<Connection>& rCxn)
        { return 0 != aDeadConnections.count(rCxn->msModelId); });

    // erase dead points
    std::erase_if(maPoints, [&aDeadPoints](const rtl::Reference<Point>& rPoint)
        { return 0 != aDeadPoints.count(rPoint->msModelId); });

    // The removal leaves holes in the sibling orders and in the presentation style indexes.
    correctConnectionOrders(maConnections);
    correctPresentationStyleIndexes(maPoints);

    // prepare retval, OOXData and OOXLayout is changed
    aRetval.push_back(DomMapFlag::OOXData);
    aRetval.push_back(DomMapFlag::OOXDrawing);
    aRetval.push_back(DomMapFlag::OOXDataImageRels);
    aRetval.push_back(DomMapFlag::OOXDataHlinkRels);
    aRetval.push_back(DomMapFlag::OOXDrawingImageRels);
    aRetval.push_back(DomMapFlag::OOXDrawingHlinkRels);

    return aRetval;
}

namespace
{
/// Which of the three data Points of a node a presentation Point is associated with
enum class PresentationAssociation
{
    Node,
    ParTrans,
    SibTrans
};

/** What one presentation Point of a node draws and where it hangs. A layout draws a node with one
    of these or with several, and it may put the connector of a node on the parTrans Point or the
    separator behind it on the sibTrans Point. A node that is already there is read off into a set
    of these, and a node that is added is drawn the way they say.
 */
struct PresentationRole
{
    /* which of the three data Points of the node it is associated with */
    PresentationAssociation meAssociation = PresentationAssociation::Node;
    /* the presentation Point it was read from */
    OUString msTemplateModelId;
    /* the name of the layout node that draws it, XML_presName */
    OUString msPresName;
    /* XML_presStyleLbl, empty when the layout names none */
    OUString msStyleLabel;
    /* the presentation Point it hangs under */
    OUString msContainerId;
    /* the name of the layout node of that container */
    OUString msContainerPresName;
    /* the layout that made it, carried by the Connections as XML_presId */
    OUString msLayoutId;
    /* XML_presStyleIdx and XML_presStyleCnt, -1 for a Point that carries neither */
    sal_Int32 mnStyleIndex = -1;
    sal_Int32 mnStyleCount = -1;
    /* where it hangs among what the container holds */
    sal_Int32 mnOrderInContainer = 0;
    /* XML_srcOrd of the presOf Connection that reaches it */
    sal_Int32 mnPresOfSourceOrder = 0;
    /* XML_bulletEnabled */
    bool mbBulletEnabled = false;
    /* whether a presOf Connection reaches it at all, which a container has none of */
    bool mbHasPresOf = false;
};

typedef std::vector< PresentationRole > PresentationRoles;

// Reads the presentation Points that belong to one node, sorted the way they hang in their
// container. rParTransId and rSibTransId are the transition Points of the Connection that leads to
// the node, either of them may be empty.
PresentationRoles readPresentationRoles(const Points& rPoints, const Connections& rConnections,
                                        std::u16string_view rNodeId, std::u16string_view rParTransId,
                                        std::u16string_view rSibTransId)
{
    PresentationRoles aRetval;

    for (const rtl::Reference<Point>& rPoint : rPoints)
    {
        if (TypeConstant::XML_pres != rPoint->mnXMLType
            || rPoint->msPresentationAssociationId.isEmpty())
            continue;

        PresentationRole aRole;

        if (rPoint->msPresentationAssociationId == rNodeId)
            aRole.meAssociation = PresentationAssociation::Node;
        else if (!rParTransId.empty() && rPoint->msPresentationAssociationId == rParTransId)
            aRole.meAssociation = PresentationAssociation::ParTrans;
        else if (!rSibTransId.empty() && rPoint->msPresentationAssociationId == rSibTransId)
            aRole.meAssociation = PresentationAssociation::SibTrans;
        else
            continue;

        aRole.msTemplateModelId = rPoint->msModelId;
        aRole.msPresName = rPoint->msPresentationLayoutName;
        aRole.msStyleLabel = rPoint->msPresentationLayoutStyleLabel;
        aRole.mnStyleIndex = rPoint->mnLayoutStyleIndex;
        aRole.mnStyleCount = rPoint->mnLayoutStyleCount;
        aRole.mbBulletEnabled = rPoint->mbBulletEnabled;

        for (const rtl::Reference<Connection>& rCxn : rConnections)
        {
            if (TypeConstant::XML_presParOf == rCxn->mnXMLType
                && rCxn->msDestId == rPoint->msModelId)
            {
                aRole.msContainerId = rCxn->msSourceId;
                aRole.mnOrderInContainer = rCxn->mnSourceOrder;
                aRole.msLayoutId = rCxn->msPresId;
            }
            else if (TypeConstant::XML_presOf == rCxn->mnXMLType
                     && rCxn->msDestId == rPoint->msModelId)
            {
                aRole.mbHasPresOf = true;
                aRole.mnPresOfSourceOrder = rCxn->mnSourceOrder;

                if (aRole.msLayoutId.isEmpty())
                    aRole.msLayoutId = rCxn->msPresId;
            }
        }

        // a presentation Point that hangs nowhere gives no place to put a copy of it
        if (aRole.msContainerId.isEmpty())
            continue;

        for (const rtl::Reference<Point>& rContainer : rPoints)
            if (rContainer->msModelId == aRole.msContainerId)
            {
                aRole.msContainerPresName = rContainer->msPresentationLayoutName;
                break;
            }

        aRetval.push_back(aRole);
    }

    std::sort(aRetval.begin(), aRetval.end(),
              [](const PresentationRole& rA, const PresentationRole& rB)
              { return rA.mnOrderInContainer < rB.mnOrderInContainer; });

    return aRetval;
}


// Fills rTarget with the data nodes below rParentId in the order the Diagram walks them: a node,
// then the nodes below that node, then the next node beside it.
void readNodesInDocumentOrder(const Connections& rConnections, std::u16string_view rParentId,
                             std::vector<OUString>& rTarget)
{
    std::vector<const Connection*> aChildren;

    for (const rtl::Reference<Connection>& rCxn : rConnections)
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msSourceId == rParentId)
            aChildren.push_back(rCxn.get());

    std::sort(aChildren.begin(), aChildren.end(),
              [](const Connection* pA, const Connection* pB)
              { return pA->mnSourceOrder < pB->mnSourceOrder; });

    for (const Connection* pCxn : aChildren)
    {
        rTarget.push_back(pCxn->msDestId);
        readNodesInDocumentOrder(rConnections, pCxn->msDestId, rTarget);
    }
}

// The data node that a presentation Point belongs to. That is the Point it is associated with when
// that is a node, and the node the transition Point leads to when it is a transition Point.
OUString readOwningNode(const Points& rPoints, const Connections& rConnections,
                        const Point& rPresentationPoint)
{
    const OUString& rAssociation(rPresentationPoint.msPresentationAssociationId);

    if (rAssociation.isEmpty())
        return OUString();

    for (const rtl::Reference<Point>& rPoint : rPoints)
    {
        if (rPoint->msModelId != rAssociation)
            continue;

        if (TypeConstant::XML_parTrans != rPoint->mnXMLType
            && TypeConstant::XML_sibTrans != rPoint->mnXMLType)
            return rAssociation;

        for (const rtl::Reference<Connection>& rCxn : rConnections)
            if (rCxn->msModelId == rPoint->msCnxId)
                return rCxn->msDestId;

        return OUString();
    }

    return OUString();
}
}

namespace
{
// The Connection that leads to the node that comes first under the given parent, nullptr when the
// parent holds no node.
const Connection* readFirstNodeUnder(const Connections& rConnections, std::u16string_view rParentId)
{
    const Connection* pRetval(nullptr);

    for (const rtl::Reference<Connection>& rCxn : rConnections)
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msSourceId == rParentId
            && (nullptr == pRetval || rCxn->mnSourceOrder < pRetval->mnSourceOrder))
            pRetval = rCxn.get();

    return pRetval;
}
}

AddedDiagramNode DiagramData_svx::addDiagramNode(std::u16string_view rTargetNode, bool bAsChild)
{
    const rtl::Reference<Point> xRoot(getRootPoint());

    if (!xRoot.is())
        return AddedDiagramNode();

    // A ModelId that leads to no node of the Diagram gives nothing to go by, and neither does an
    // empty one. The shape that draws the Diagram as a whole is such a ModelId, so is the one of a
    // shape that draws the step from one node to the next.
    rtl::Reference<Point> xTarget(rTargetNode.empty() ? nullptr
                                                        : getDataNodeForModelID(rTargetNode));

    if (xTarget.is() && TypeConstant::XML_node != xTarget->mnXMLType)
        xTarget = nullptr;

    // With nothing to go by the node that comes first at the top level answers instead: the new
    // node goes in front of it and is drawn the way it is.
    if (!xTarget.is())
    {
        const Connection* pFirstNode(readFirstNodeUnder(maConnections, xRoot->msModelId));

        if (nullptr == pFirstNode)
        {
            SAL_WARN("svx.diagram", "addDiagramNode: the Diagram holds no node to copy from");
            return AddedDiagramNode();
        }

        return insertDiagramNode(xRoot->msModelId, 0, pFirstNode->msDestId);
    }

    // where the node to go by hangs, and the place it holds there
    OUString aParentId;
    sal_Int32 nTargetSourceOrder(0);

    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msDestId == xTarget->msModelId)
        {
            aParentId = rCxn->msSourceId;
            nTargetSourceOrder = rCxn->mnSourceOrder;
        }

    if (aParentId.isEmpty())
    {
        SAL_WARN("svx.diagram", "addDiagramNode: the node to go by hangs under no parent");
        return AddedDiagramNode();
    }

    // A node that hangs below another node takes the new node beside itself, right behind it,
    // whether or not a node below it was asked for. So does a node at the top level when no node
    // below it was asked for. Either way it is the template as well.
    const bool bTargetIsAtTopLevel(aParentId == xRoot->msModelId);

    if (!bAsChild || !bTargetIsAtTopLevel)
        return insertDiagramNode(aParentId, nTargetSourceOrder + 1, xTarget->msModelId);

    // The new node goes below the node to go by, as the first of the nodes there. The node that
    // comes first below it is the template, and when it holds none, the node that comes first
    // below the first node at the top level that holds any.
    const Connection* pTemplate(readFirstNodeUnder(maConnections, xTarget->msModelId));

    if (nullptr == pTemplate)
    {
        // the node that comes first at the top level and holds nodes below it
        const Connection* pHolder(nullptr);

        for (const rtl::Reference<Connection>& rCxn : maConnections)
        {
            if (TypeConstant::XML_parOf != rCxn->mnXMLType || rCxn->msSourceId != xRoot->msModelId)
                continue;

            if (nullptr != pHolder && rCxn->mnSourceOrder > pHolder->mnSourceOrder)
                continue;

            if (nullptr != readFirstNodeUnder(maConnections, rCxn->msDestId))
                pHolder = rCxn.get();
        }

        if (nullptr != pHolder)
            pTemplate = readFirstNodeUnder(maConnections, pHolder->msDestId);
    }

    if (nullptr == pTemplate)
    {
        SAL_WARN("svx.diagram", "addDiagramNode: no node below a node to copy from");
        return AddedDiagramNode();
    }

    return insertDiagramNode(xTarget->msModelId, 0, pTemplate->msDestId);
}

AddedDiagramNode DiagramData_svx::insertDiagramNode(std::u16string_view rParentId,
                                                    sal_Int32 nNewSourceOrder,
                                                    std::u16string_view rTemplateNodeId)
{
    DomMapFlags aChangedParts;

    const rtl::Reference<Point> xParent(getDataNodeForModelID(rParentId));
    const rtl::Reference<Point> xTemplate(getDataNodeForModelID(rTemplateNodeId));

    if (!xParent.is() || !xTemplate.is() || TypeConstant::XML_doc == xTemplate->mnXMLType)
    {
        SAL_WARN("svx.diagram", "insertDiagramNode: the given ModelId leads to no data node");
        return AddedDiagramNode();
    }

    const OUString aParentId(xParent->msModelId);
    const OUString aTemplateNodeId(xTemplate->msModelId);

    // The Connection that holds the template names its parent and its two transition Points, and
    // it says where the template sits among the nodes beside it.
    OUString aTemplateParentId;
    OUString aTemplateParTransId;
    OUString aTemplateSibTransId;
    sal_Int32 nTemplateSourceOrder(0);
    bool bTemplateFound(false);

    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msDestId == aTemplateNodeId)
        {
            aTemplateParentId = rCxn->msSourceId;
            aTemplateParTransId = rCxn->msParTransId;
            aTemplateSibTransId = rCxn->msSibTransId;
            nTemplateSourceOrder = rCxn->mnSourceOrder;
            bTemplateFound = true;
        }

    if (!bTemplateFound)
    {
        SAL_WARN("svx.diagram", "insertDiagramNode: the template node hangs under no parent");
        return AddedDiagramNode();
    }

    // The template hangs under another parent than the new node, so the presentation Points of the
    // new node go into the containers that belong to its own parent, not into the ones the
    // template hangs in.
    const bool bTemplateUnderOtherParent(aTemplateParentId != aParentId);

    // whether the new node ends up in front of the template, which only a template beside it can
    // be asked about
    const bool bInFront(!bTemplateUnderOtherParent && nNewSourceOrder <= nTemplateSourceOrder);

    sal_Int32 nHighestSourceOrder(-1);

    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msSourceId == aParentId
            && rCxn->mnSourceOrder > nHighestSourceOrder)
            nHighestSourceOrder = rCxn->mnSourceOrder;

    const bool bNewNodeIsLast(nNewSourceOrder > nHighestSourceOrder);

    // what the new node is to be drawn with, which is what the template is drawn with
    PresentationRoles aRoles(readPresentationRoles(maPoints, maConnections, aTemplateNodeId,
                                                   aTemplateParTransId, aTemplateSibTransId));

    if (aRoles.empty())
    {
        SAL_WARN("svx.diagram", "insertDiagramNode: the template node has no presentation Point");
        return AddedDiagramNode();
    }

    bool bTemplateHoldsSeparator(false);

    for (const PresentationRole& rRole : aRoles)
        if (PresentationAssociation::SibTrans == rRole.meAssociation)
            bTemplateHoldsSeparator = true;

    // A layout that draws a separator behind every node but the last one leaves the node that
    // comes last without one. The new node takes that place, so it does not get one either, and
    // the node it goes behind now needs the separator it did not have.
    PresentationRole aSeparatorForTemplate;
    bool bSeparatorForTemplate(false);

    if (bNewNodeIsLast && !bTemplateHoldsSeparator && !aTemplateSibTransId.isEmpty()
        && !bTemplateUnderOtherParent)
        for (const rtl::Reference<Connection>& rCxn : maConnections)
        {
            if (TypeConstant::XML_parOf != rCxn->mnXMLType || rCxn->msSourceId != aParentId
                || rCxn->msDestId == aTemplateNodeId || rCxn->msSibTransId.isEmpty())
                continue;

            for (const PresentationRole& rRole : readPresentationRoles(
                     maPoints, maConnections, rCxn->msDestId, EMPTY_OUSTRING, rCxn->msSibTransId))
                if (PresentationAssociation::SibTrans == rRole.meAssociation)
                {
                    aSeparatorForTemplate = rRole;
                    bSeparatorForTemplate = true;
                    break;
                }

            if (bSeparatorForTemplate)
                break;
        }

    // The new node comes last under a parent that held nothing, and the template was taken from
    // somewhere else. When the layout leaves the node that comes last without a separator, the
    // copy does not take the one the template holds.
    if (bTemplateUnderOtherParent && bTemplateHoldsSeparator)
    {
        // The nodes beside the template answer whether the layout leaves the last node without a
        // separator.
        const Connection* pLastBesideTemplate(nullptr);

        for (const rtl::Reference<Connection>& rCxn : maConnections)
            if (TypeConstant::XML_parOf == rCxn->mnXMLType
                && rCxn->msSourceId == aTemplateParentId
                && (nullptr == pLastBesideTemplate
                    || rCxn->mnSourceOrder > pLastBesideTemplate->mnSourceOrder))
                pLastBesideTemplate = rCxn.get();

        bool bLastHoldsSeparator(false);

        if (nullptr != pLastBesideTemplate)
            for (const PresentationRole& rRole :
                 readPresentationRoles(maPoints, maConnections, pLastBesideTemplate->msDestId,
                                       EMPTY_OUSTRING, pLastBesideTemplate->msSibTransId))
                if (PresentationAssociation::SibTrans == rRole.meAssociation)
                    bLastHoldsSeparator = true;

        if (!bLastHoldsSeparator)
            std::erase_if(aRoles, [](const PresentationRole& rRole)
                          { return PresentationAssociation::SibTrans == rRole.meAssociation; });
    }

    // the container each new presentation Point hangs under
    if (bTemplateUnderOtherParent)
        for (PresentationRole& rRole : aRoles)
        {
            OUString aOwnContainerId;

            for (const rtl::Reference<Point>& rPoint : maPoints)
                if (TypeConstant::XML_pres == rPoint->mnXMLType
                    && rPoint->msPresentationAssociationId == aParentId
                    && rPoint->msPresentationLayoutName == rRole.msContainerPresName)
                    aOwnContainerId = rPoint->msModelId;

            if (aOwnContainerId.isEmpty())
            {
                SAL_WARN("svx.diagram",
                         "addDiagramNode: the parent holds no container of that name");
                return AddedDiagramNode();
            }

            rRole.msContainerId = aOwnContainerId;
        }

    const OUString aNewNodeId(createModelId());
    const OUString aNewParTransId(createModelId());
    const OUString aNewSibTransId(createModelId());
    const OUString aNewParOfId(createModelId());

    // One new presentation Point per role, and the separator that the template may be owed comes
    // first because it sits between the template and the new node.
    struct NewPresentationPoint
    {
        OUString msModelId;
        OUString msAssociationId;
        OUString msContainerId;
        const PresentationRole* mpRole;
    };
    std::vector<NewPresentationPoint> aNewPoints;

    if (bSeparatorForTemplate)
        aNewPoints.push_back({ createModelId(), aTemplateSibTransId,
                               aSeparatorForTemplate.msContainerId, &aSeparatorForTemplate });

    for (const PresentationRole& rRole : aRoles)
    {
        OUString aAssociationId(aNewNodeId);

        if (PresentationAssociation::ParTrans == rRole.meAssociation)
            aAssociationId = aNewParTransId;
        else if (PresentationAssociation::SibTrans == rRole.meAssociation)
            aAssociationId = aNewSibTransId;

        aNewPoints.push_back({ createModelId(), aAssociationId, rRole.msContainerId, &rRole });
    }

    // A node brings its own containers along when it is drawn with any, as a node that can hold
    // nodes below it is. What hung inside such a container goes inside the copy of it, so that the
    // new node holds its own and does not reach into the ones the template hangs in.
    std::unordered_set<OUString> aFreshContainers;

    for (NewPresentationPoint& rNew : aNewPoints)
        for (const NewPresentationPoint& rOther : aNewPoints)
            if (rOther.mpRole->msTemplateModelId == rNew.msContainerId)
            {
                rNew.msContainerId = rOther.msModelId;
                aFreshContainers.insert(rOther.msModelId);
                break;
            }

    std::map<OUString, std::vector<NewPresentationPoint>> aNewPointsForContainer;

    for (const NewPresentationPoint& rNew : aNewPoints)
        aNewPointsForContainer[rNew.msContainerId].push_back(rNew);

    // where that block starts inside each container, and by how much the container grows
    std::map<OUString, sal_Int32> aInsertOrderForContainer;
    std::map<OUString, sal_Int32> aGrowthForContainer;

    for (const auto& rEntry : aNewPointsForContainer)
    {
        sal_Int32 nInsertOrder(0);

        if (aFreshContainers.count(rEntry.first))
        {
            // the container is created here and holds nothing else, so the block keeps the order
            // it had inside the container it was copied from
            nInsertOrder = rEntry.second.front().mpRole->mnOrderInContainer;
        }
        else if (bTemplateUnderOtherParent)
        {
            // the container belongs to a parent that held nothing, so the block goes behind
            // whatever else hangs there
            nInsertOrder = 0;

            for (const rtl::Reference<Connection>& rCxn : maConnections)
                if (TypeConstant::XML_presParOf == rCxn->mnXMLType
                    && rCxn->msSourceId == rEntry.first && rCxn->mnSourceOrder >= nInsertOrder)
                    nInsertOrder = rCxn->mnSourceOrder + 1;
        }
        else
        {
            bool bFirst(true);

            for (const NewPresentationPoint& rNew : rEntry.second)
            {
                const sal_Int32 nCandidate(bInFront ? rNew.mpRole->mnOrderInContainer
                                                        : rNew.mpRole->mnOrderInContainer + 1);

                if (bFirst || (bInFront ? nCandidate < nInsertOrder
                                            : nCandidate > nInsertOrder))
                    nInsertOrder = nCandidate;

                bFirst = false;
            }
        }

        aInsertOrderForContainer[rEntry.first] = nInsertOrder;
        aGrowthForContainer[rEntry.first] = static_cast<sal_Int32>(rEntry.second.size());
    }

    // the style index the new node takes, per style label
    std::map<OUString, sal_Int32> aNewIndexForLabel;

    for (const PresentationRole& rRole : aRoles)
    {
        if (rRole.msStyleLabel.isEmpty() || rRole.mnStyleIndex < 0)
            continue;

        if (!bTemplateUnderOtherParent)
        {
            aNewIndexForLabel[rRole.msStyleLabel]
                = bInFront ? rRole.mnStyleIndex : rRole.mnStyleIndex + 1;
            continue;
        }

        // The style indexes run through the Diagram in the order it walks its nodes, and the new
        // node follows its parent. So the index is the number of index values of that label that
        // belong to nodes the walk reaches no later than the parent.
        std::vector<OUString> aWalk;
        rtl::Reference<Point> xRoot(getRootPoint());

        if (xRoot.is())
            readNodesInDocumentOrder(maConnections, xRoot->msModelId, aWalk);

        size_t nParentAt(aWalk.size());

        for (size_t a(0); a < aWalk.size(); a++)
            if (aWalk[a] == aParentId)
                nParentAt = a;

        std::set<sal_Int32> aIndexesBefore;

        for (const rtl::Reference<Point>& rPoint : maPoints)
        {
            if (TypeConstant::XML_pres != rPoint->mnXMLType
                || rPoint->msPresentationLayoutStyleLabel != rRole.msStyleLabel
                || rPoint->mnLayoutStyleIndex < 0)
                continue;

            const OUString aOwnerId(readOwningNode(maPoints, maConnections, *rPoint));

            for (size_t a(0); a < aWalk.size() && a <= nParentAt; a++)
                if (aWalk[a] == aOwnerId)
                    aIndexesBefore.insert(rPoint->mnLayoutStyleIndex);
        }

        aNewIndexForLabel[rRole.msStyleLabel] = static_cast<sal_Int32>(aIndexesBefore.size());
    }

    // Make room: among the nodes beside the new one, inside every container that takes new
    // presentation Points, and among the style indexes of every label the new node carries.
    for (const rtl::Reference<Connection>& rCxn : maConnections)
    {
        if (TypeConstant::XML_parOf == rCxn->mnXMLType && rCxn->msSourceId == aParentId
            && rCxn->mnSourceOrder >= nNewSourceOrder)
        {
            rCxn->mnSourceOrder++;
            continue;
        }

        if (TypeConstant::XML_presParOf != rCxn->mnXMLType)
            continue;

        const auto aFound(aInsertOrderForContainer.find(rCxn->msSourceId));

        if (aFound != aInsertOrderForContainer.end() && rCxn->mnSourceOrder >= aFound->second)
            rCxn->mnSourceOrder += aGrowthForContainer[rCxn->msSourceId];
    }

    for (const auto& rEntry : aNewIndexForLabel)
        for (const rtl::Reference<Point>& rPoint : maPoints)
            if (rPoint->msPresentationLayoutStyleLabel == rEntry.first
                && rPoint->mnLayoutStyleIndex >= rEntry.second)
                rPoint->mnLayoutStyleIndex++;

    // the Connection that holds the new node, and the two transition Points that hang off it
    Connection& rNewParOf(appendConnection(maConnections, TypeConstant::XML_parOf, aNewParOfId,
                                           aParentId, aNewNodeId, nNewSourceOrder, OUString()));
    rNewParOf.msParTransId = aNewParTransId;
    rNewParOf.msSibTransId = aNewSibTransId;

    rtl::Reference<Point> xNewNode(new Point);
    xNewNode->mnXMLType = TypeConstant::XML_node;
    xNewNode->msModelId = aNewNodeId;
    xNewNode->msPlaceholderText = u"[Text]"_ustr;

    rtl::Reference<Point> xNewParTrans(new Point);
    xNewParTrans->mnXMLType = TypeConstant::XML_parTrans;
    xNewParTrans->msModelId = aNewParTransId;
    xNewParTrans->msCnxId = aNewParOfId;

    rtl::Reference<Point> xNewSibTrans(new Point);
    xNewSibTrans->mnXMLType = TypeConstant::XML_sibTrans;
    xNewSibTrans->msModelId = aNewSibTransId;
    xNewSibTrans->msCnxId = aNewParOfId;

    maPoints.push_back(xNewNode);
    maPoints.push_back(xNewParTrans);
    maPoints.push_back(xNewSibTrans);

    for (const auto& rEntry : aNewPointsForContainer)
    {
        sal_Int32 nOrder(aInsertOrderForContainer[rEntry.first]);

        for (const NewPresentationPoint& rNew : rEntry.second)
        {
            const PresentationRole& rRole(*rNew.mpRole);

            rtl::Reference<Point> xNewPresentation(new Point);
            xNewPresentation->mnXMLType = TypeConstant::XML_pres;
            xNewPresentation->msModelId = rNew.msModelId;
            xNewPresentation->msPresentationAssociationId = rNew.msAssociationId;
            xNewPresentation->msPresentationLayoutName = rRole.msPresName;
            xNewPresentation->msPresentationLayoutStyleLabel = rRole.msStyleLabel;
            xNewPresentation->mnLayoutStyleCount = rRole.mnStyleCount;
            xNewPresentation->mbBulletEnabled = rRole.mbBulletEnabled;

            const auto aFound(aNewIndexForLabel.find(rRole.msStyleLabel));
            xNewPresentation->mnLayoutStyleIndex
                = (aFound == aNewIndexForLabel.end()) ? -1 : aFound->second;

            maPoints.push_back(xNewPresentation);

            appendConnection(maConnections, TypeConstant::XML_presParOf, createModelId(),
                             rEntry.first, rNew.msModelId, nOrder++, rRole.msLayoutId);

            if (rRole.mbHasPresOf)
                appendConnection(maConnections, TypeConstant::XML_presOf, createModelId(),
                                 rNew.msAssociationId, rNew.msModelId,
                                 rRole.mnPresOfSourceOrder, rRole.msLayoutId);
        }
    }

    // The added entries were given the orders they need, this only closes what is left over.
    correctConnectionOrders(maConnections);
    correctPresentationStyleIndexes(maPoints);
    sortPresentationPointsIntoDrawingOrder(maPoints, maConnections);

    // prepare retval, OOXData and OOXLayout is changed
    aChangedParts.push_back(DomMapFlag::OOXData);
    aChangedParts.push_back(DomMapFlag::OOXDrawing);
    aChangedParts.push_back(DomMapFlag::OOXDataImageRels);
    aChangedParts.push_back(DomMapFlag::OOXDataHlinkRels);
    aChangedParts.push_back(DomMapFlag::OOXDrawingImageRels);
    aChangedParts.push_back(DomMapFlag::OOXDrawingHlinkRels);

    return { aNewNodeId, aTemplateNodeId, aChangedParts };
}

DiagramDataState::DiagramDataState(const Connections& aConnections, const Points& aPoints, const uno::Reference< drawing::XShape >& rRootShape)
: maConnections(copyConnections(aConnections))
, maPoints(copyPoints(aPoints))
, mxShapes()
, maShapeTransformations()
, maTransformation()
{
    SdrObjGroup* pSource(dynamic_cast<SdrObjGroup*>(SdrObject::getSdrObjectFromXShape(rRootShape)));
    if (nullptr != pSource)
    {
        basegfx::B2DPolyPolygon aPolyPolygon;
        pSource->TRGetBaseGeometry(maTransformation, aPolyPolygon);

        for(size_t a(0); a < pSource->GetObjCount(); a++)
        {
            SdrObject* pCandidate(pSource->GetObj(a));

            if (nullptr != pCandidate)
            {
                uno::Reference<drawing::XShape> xCandidate(pCandidate->getUnoShape());
                mxShapes.push_back(xCandidate);

                basegfx::B2DHomMatrix aCandidateTransformation;
                basegfx::B2DPolyPolygon aCandidatePolyPolygon;
                pCandidate->TRGetBaseGeometry(aCandidateTransformation, aCandidatePolyPolygon);
                maShapeTransformations.push_back(aCandidateTransformation);
            }
        }
    }
}

DiagramDataStatePtr DiagramData_svx::extractDiagramDataState() const
{
    // Just copy all Connections && Points. The shared_ptr data in
    // Point-entries is no problem, it just continues exiting shared
    return std::make_shared< DiagramDataState >(maConnections, maPoints, accessRootShape());
}

void DiagramData_svx::applyDiagramDataState(const DiagramDataStatePtr& rState)
{
    if(rState)
    {
        // The state is what an undo goes back to, copy them.
        maConnections = copyConnections(rState->getConnections());
        maPoints = copyPoints(rState->getPoints());

        uno::Reference<drawing::XShapes> xRootShape(accessRootShape(), uno::UNO_QUERY);
        if (xRootShape.is())
        {
            SdrObjGroup* pTarget(dynamic_cast<SdrObjGroup*>(SdrObject::getSdrObjectFromXShape(xRootShape)));

            if (nullptr != pTarget)
            {
                // Delete all existing shapes in that group
                pTarget->getChildrenOfSdrObject()->ClearSdrObjList();
            }

            const std::vector<uno::Reference<drawing::XShape>>& rXShapes(rState->getXShapes());
            for (auto& rShape : rXShapes)
                xRootShape->add(rShape);

            basegfx::B2DPolyPolygon aPolyPolygon;
            pTarget->TRSetBaseGeometry(rState->getTransformation(), aPolyPolygon);

            // Each shape shall cover again what it covered before. This comes after
            // the Group getting it set, because that one reaches every shape inside it.
            const std::vector<basegfx::B2DHomMatrix>& rShapeTransformations(
                rState->getShapeTransformations());

            for (size_t a(0); a < rXShapes.size() && a < rShapeTransformations.size(); a++)
            {
                SdrObject* pShape(SdrObject::getSdrObjectFromXShape(rXShapes[a]));

                if (nullptr != pShape)
                {
                    basegfx::B2DPolyPolygon aShapePolyPolygon;
                    pShape->TRSetBaseGeometry(rShapeTransformations[a], aShapePolyPolygon);
                }
            }
        }

        // Reset temporary buffered ModelData association lists & rebuild them
        // and the Diagram DataModel. Do that here *immediately* to prevent
        // re-usage of potentially invalid Connection/Point objects
        buildDiagramDataModel(true);
    }
}

void DiagramData_svx::getDiagramChildrenString(
    OUStringBuffer& rBuf,
    const rtl::Reference<svx::diagram::Point>& xPoint,
    sal_Int32 nLevel) const
{
    if (!xPoint.is())
        return;

    if (nLevel > 0)
    {
        // One dash for each step down from the top level. Before a tab was used, but that
        // cannot be typed into an edit field in a dialog, would select next control
        for (sal_Int32 i = 0; i < nLevel-1; i++)
            rBuf.append('-');
        rBuf.append('+');
        rBuf.append(' ');
        const OUString aText(getTextForPoint(*xPoint));
        rBuf.append(aText);
        rBuf.append('\n');
    }

    Points aChildren;
    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (rCxn->mnXMLType == TypeConstant::XML_parOf && rCxn->msSourceId == xPoint->msModelId)
        {
            if (rCxn->mnSourceOrder >= static_cast<sal_Int32>(aChildren.size()))
                aChildren.resize(rCxn->mnSourceOrder + 1);
            rtl::Reference<Point> xChild(getPointByModelID(rCxn->msDestId));
            if (xChild.is())
                aChildren[rCxn->mnSourceOrder] = xChild;
        }

    for (const rtl::Reference<Point>& rChild : aChildren)
        getDiagramChildrenString(rBuf, rChild, nLevel + 1);
}

uno::Reference<drawing::XShape> DiagramData_svx::getXShapeByModelID(std::u16string_view rModelID) const
{
    uno::Reference<drawing::XShape> xRetval;

    SdrObject* pCandidate(SdrObject::getSdrObjectFromXShape(accessRootShape()));
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

rtl::Reference<Point> DiagramData_svx::getPointByModelID(std::u16string_view rModelID) const
{
    for (const rtl::Reference<Point>& rCandidate : getPoints())
        if (rModelID == rCandidate->msModelId)
            return rCandidate;

    return nullptr;
}

uno::Reference<drawing::XShape> DiagramData_svx::getMasterXShapeForPoint(const Point& rPoint) const
{
    for (const rtl::Reference<Point>& rCandidate : getPoints())
    {
        if (!rCandidate->msPresentationAssociationId.isEmpty()
            && rCandidate->msPresentationAssociationId == rPoint.msModelId)
        {
            const uno::Reference<drawing::XShape> xMasterText(
                getXShapeByModelID(rCandidate->msModelId));
            if (xMasterText)
                return xMasterText;
        }
    }

    return uno::Reference<drawing::XShape>();
}

OUString DiagramData_svx::getTextForPoint(const Point& rPoint) const
{
    uno::Reference<drawing::XShape> xMasterText(getMasterXShapeForPoint(rPoint));
    uno::Reference<text::XText> xText(xMasterText, uno::UNO_QUERY);

    if (xText)
        return xText->getString();

    return OUString();
}

std::vector<std::pair<OUString, OUString>> DiagramData_svx::getDiagramChildren(const OUString& rParentId) const
{
    const OUString sModelId = rParentId.isEmpty() ? getRootPoint()->msModelId : rParentId;
    std::vector<std::pair<OUString, OUString>> aChildren;
    for (const rtl::Reference<Connection>& rCxn : maConnections)
        if (rCxn->mnXMLType == TypeConstant::XML_parOf && rCxn->msSourceId == sModelId)
        {
            if (rCxn->mnSourceOrder >= static_cast<sal_Int32>(aChildren.size()))
                aChildren.resize(rCxn->mnSourceOrder + 1);
            rtl::Reference<Point> xChild(getPointByModelID(rCxn->msDestId));
            if (xChild.is())
            {
                const OUString aText(getTextForPoint(*xChild));
                aChildren[rCxn->mnSourceOrder] = std::make_pair(
                    xChild->msModelId,
                    aText);
            }
        }

    // HACK: empty items shouldn't appear there
    std::erase_if(aChildren, [](const std::pair<OUString, OUString>& aItem) { return aItem.first.isEmpty(); });

    return aChildren;
}

void DiagramData_svx::addConnection(svx::diagram::TypeConstant nType, const OUString& sSourceId, const OUString& sDestId)
{
    sal_Int32 nMaxOrd = -1;
    for (const rtl::Reference<Connection>& aCxn : maConnections)
        if (aCxn->mnXMLType == nType && aCxn->msSourceId == sSourceId)
            nMaxOrd = std::max(nMaxOrd, aCxn->mnSourceOrder);

    svx::diagram::Connection& rCxn(*maConnections.emplace_back(new svx::diagram::Connection));
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
    for (const rtl::Reference<Connection>& elem : rCnx)
    {
        if( !elem->msParTransId.isEmpty() &&
            !elem->msSibTransId.isEmpty() &&
            !elem->msSourceId.isEmpty() &&
            !elem->msDestId.isEmpty() &&
            elem->mnXMLType == TypeConstant::XML_parOf &&
            rNodeName == elem->msDestId )
        {
            return calcDepth(elem->msSourceId, rCnx) + 1;
        }
    }

    return 0;
}

void DiagramData_svx::buildDiagramDataModel(bool /*bClearOoxShapes*/)
{
    // build name-object maps
    maPointsPresNameMap.clear();
    maConnectionNameMap.clear();
    maPresOfNameMap.clear();

#ifdef DEBUG_OOX_DIAGRAM
    std::ofstream output("tree.dot");

    output << "digraph datatree {" << std::endl;
#endif
    svx::diagram::Points& rPoints = getPoints();
    for (const rtl::Reference<Point>& point : rPoints)
    {
#ifdef DEBUG_OOX_DIAGRAM
        output << "\t"
               << normalizeDotName(point->msModelId).getStr()
               << "[";

        if( !point->msPresentationLayoutName.isEmpty() )
            output << "label=\""
                   << OUStringToOString(
                       point->msPresentationLayoutName,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";
        else
            output << "label=\""
                   << OUStringToOString(
                       point->msModelId,
                       RTL_TEXTENCODING_UTF8).getStr() << "\", ";

        switch( point->mnXMLType )
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

        // does currpoint have any text set?
        const OUString aTextAtPoint(getTextForPoint(*point));
        if(!aTextAtPoint.isEmpty())
        {
#ifdef DEBUG_OOX_DIAGRAM
            static sal_Int32 nCount=0;
            output << "\t"
                   << "textNode" << nCount
                   << " ["
                   << "label=\""
                   << OUStringToOString(
                       aTextAtPoint,
                       RTL_TEXTENCODING_UTF8).getStr()
                   << "\"" << "];" << std::endl;
            output << "\t"
                   << normalizeDotName(point->msModelId).getStr()
                   << " -> "
                   << "textNode" << nCount++
                   << ";" << std::endl;
#endif
        }

        const bool bInserted1(nullptr != getPointByModelID(point->msModelId));
        SAL_WARN_IF(!bInserted1, "oox.drawingml", "DiagramData_svx::build(): non-unique point model id");

        if( !point->msPresentationLayoutName.isEmpty() )
        {
            DiagramData_svx::PointsNameMap::value_type::second_type& rVec=
                getPointsPresNameMap()[point->msPresentationLayoutName];
            rVec.push_back(point);
        }
    }

    // The presentation Points of a layout node are in the order the Diagram draws them, which is
    // the order the presParOf Connections defines. If already in that order, nothing changes. But
    // we cannot be sure that import and source of Points provides that condition
    {
        const std::map<OUString, size_t> aDrawingOrder(
            readDrawingOrderOfPresentationPoints(maPoints, maConnections));

        for (auto& rEntry : maPointsPresNameMap)
            std::stable_sort(rEntry.second.begin(), rEntry.second.end(),
                             [&aDrawingOrder](const rtl::Reference<Point>& rA,
                                              const rtl::Reference<Point>& rB)
                             {
                                 return readDrawingPlace(aDrawingOrder, rA->msModelId)
                                        < readDrawingPlace(aDrawingOrder, rB->msModelId);
                             });
    }

    const svx::diagram::Connections& rConnections = getConnections();
    for (const rtl::Reference<Connection>& connection : rConnections)
    {
#ifdef DEBUG_OOX_DIAGRAM
        if( !connection->msParTransId.isEmpty() ||
            !connection->msSibTransId.isEmpty() )
        {
            if( !connection->msSourceId.isEmpty() ||
                !connection->msDestId.isEmpty() )
            {
                output << "\t"
                       << normalizeDotName(connection->msSourceId).getStr()
                       << " -> "
                       << normalizeDotName(connection->msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection->msSibTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection->msDestId).getStr()
                       << " [style=dotted,"
                       << ((connection->mnXMLType == TypeConstant::XML_presOf) ? " color=red, " : ((connection->mnXMLType == TypeConstant::XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(connection->msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
            else
            {
                output << "\t"
                       << normalizeDotName(connection->msParTransId).getStr()
                       << " -> "
                       << normalizeDotName(connection->msSibTransId).getStr()
                       << " ["
                       << ((connection->mnXMLType == TypeConstant::XML_presOf) ? " color=red, " : ((connection->mnXMLType == TypeConstant::XML_presParOf) ? " color=green, " : " "))
                       << "label=\""
                       << OUStringToOString(connection->msModelId,
                                                 RTL_TEXTENCODING_UTF8 ).getStr()
                       << "\"];" << std::endl;
            }
        }
        else if( !connection->msSourceId.isEmpty() ||
                 !connection->msDestId.isEmpty() )
            output << "\t"
                   << normalizeDotName(connection->msSourceId).getStr()
                   << " -> "
                   << normalizeDotName(connection->msDestId).getStr()
                   << " [label=\""
                   << OUStringToOString(connection->msModelId,
                                             RTL_TEXTENCODING_UTF8 ).getStr()
                   << ((connection->mnXMLType == TypeConstant::XML_presOf) ? "\", color=red]" : ((connection->mnXMLType == TypeConstant::XML_presParOf) ? "\", color=green]" : "\"]"))
                   << ";" << std::endl;
#endif

        const bool bInserted1 = maConnectionNameMap.insert(
            std::make_pair(connection->msModelId, connection)).second;

        SAL_WARN_IF(!bInserted1, "oox.drawingml", "DiagramData_svx::build(): non-unique connection model id");

        if( connection->mnXMLType == TypeConstant::XML_presOf )
        {
            DiagramData_svx::StringMap::value_type::second_type& rVec = getPresOfNameMap()[connection->msDestId];
            rVec[connection->mnDestOrder] = { connection->msSourceId, sal_Int32(0) };
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
