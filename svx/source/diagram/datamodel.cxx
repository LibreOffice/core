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

#include <svx/diagram/datamodel.hxx>
#include <sal/log.hxx>

namespace svx::diagram {

Connection::Connection()
: mnXMLType( XML_none )
, mnSourceOrder( 0 )
, mnDestOrder( 0 )
{
}

Point::Point()
: mnXMLType(XML_none)
, mnMaxChildren(-1)
, mnPreferredChildren(-1)
, mnDirection(XML_norm)
, mnResizeHandles(XML_rel)
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

DiagramData::DiagramData()
{
}

DiagramData::~DiagramData()
{
}

const Point* DiagramData::getRootPoint() const
{
    for (const auto & aCurrPoint : maPoints)
        if (aCurrPoint.mnXMLType == TypeConstant::XML_doc)
            return &aCurrPoint;

    SAL_WARN("svx.diagram", "No root point");
    return nullptr;
}

OUString DiagramData::getString() const
{
    OUStringBuffer aBuf;
    const Point* pPoint = getRootPoint();
    getChildrenString(aBuf, pPoint, 0);
    return aBuf.makeStringAndClear();
}

bool DiagramData::removeNode(const OUString& rNodeId)
{
    // check if it doesn't have children
    for (const auto& aCxn : maConnections)
        if (aCxn.mnXMLType == TypeConstant::XML_parOf && aCxn.msSourceId == rNodeId)
        {
            SAL_WARN("svx.diagram", "Node has children - can't be removed");
            return false;
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
    maConnections.erase(std::remove_if(maConnections.begin(), maConnections.end(),
                                       [aIdsToRemove](const Connection& rCxn) {
                                           return aIdsToRemove.count(rCxn.msSourceId) || aIdsToRemove.count(rCxn.msDestId);
                                       }),
                        maConnections.end());

    // remove data and presentation nodes
    maPoints.erase(std::remove_if(maPoints.begin(), maPoints.end(),
                                  [aIdsToRemove](const Point& rPoint) {
                                      return aIdsToRemove.count(rPoint.msModelId);
                                  }),
                   maPoints.end());

    // TODO: fix source/dest order

    build(true);
    return true;
}

void DiagramData::addConnection(svx::diagram::TypeConstant nType, const OUString& sSourceId, const OUString& sDestId)
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
