/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawingml/chart/geographyconverter.hxx>
#include <drawingml/chart/geographymodel.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star;

namespace oox::drawingml::chart
{
namespace
{
uno::Sequence<beans::PropertyValue> addressToProps(const GeoAddressModel& rAddr)
{
    std::vector<beans::PropertyValue> aVec;
    if (rAddr.mosAddress1.has_value())
        aVec.push_back({ u"address1"_ustr, 0, uno::Any(*rAddr.mosAddress1),
                         beans::PropertyState_DIRECT_VALUE });
    if (rAddr.mosCountryRegion.has_value())
        aVec.push_back({ u"countryRegion"_ustr, 0, uno::Any(*rAddr.mosCountryRegion),
                         beans::PropertyState_DIRECT_VALUE });
    if (rAddr.mosAdminDistrict1.has_value())
        aVec.push_back({ u"adminDistrict1"_ustr, 0, uno::Any(*rAddr.mosAdminDistrict1),
                         beans::PropertyState_DIRECT_VALUE });
    if (rAddr.mosAdminDistrict2.has_value())
        aVec.push_back({ u"adminDistrict2"_ustr, 0, uno::Any(*rAddr.mosAdminDistrict2),
                         beans::PropertyState_DIRECT_VALUE });
    if (rAddr.mosPostalCode.has_value())
        aVec.push_back({ u"postalCode"_ustr, 0, uno::Any(*rAddr.mosPostalCode),
                         beans::PropertyState_DIRECT_VALUE });
    if (rAddr.mosLocality.has_value())
        aVec.push_back({ u"locality"_ustr, 0, uno::Any(*rAddr.mosLocality),
                         beans::PropertyState_DIRECT_VALUE });
    if (rAddr.mosIsoCountryCode.has_value())
        aVec.push_back({ u"isoCountryCode"_ustr, 0, uno::Any(*rAddr.mosIsoCountryCode),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoAddressModel addressFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoAddressModel aAddr;
    for (const auto& rProp : rProps)
    {
        OUString sVal;
        if (!(rProp.Value >>= sVal))
            continue;
        if (rProp.Name == "address1")
            aAddr.mosAddress1 = sVal;
        else if (rProp.Name == "countryRegion")
            aAddr.mosCountryRegion = sVal;
        else if (rProp.Name == "adminDistrict1")
            aAddr.mosAdminDistrict1 = sVal;
        else if (rProp.Name == "adminDistrict2")
            aAddr.mosAdminDistrict2 = sVal;
        else if (rProp.Name == "postalCode")
            aAddr.mosPostalCode = sVal;
        else if (rProp.Name == "locality")
            aAddr.mosLocality = sVal;
        else if (rProp.Name == "isoCountryCode")
            aAddr.mosIsoCountryCode = sVal;
    }
    return aAddr;
}

uno::Sequence<beans::PropertyValue> locationToProps(const GeoLocationModel& rLoc)
{
    std::vector<beans::PropertyValue> aVec;
    if (rLoc.mosLatitude.has_value())
        aVec.push_back({ u"latitude"_ustr, 0, uno::Any(*rLoc.mosLatitude),
                         beans::PropertyState_DIRECT_VALUE });
    if (rLoc.mosLongitude.has_value())
        aVec.push_back({ u"longitude"_ustr, 0, uno::Any(*rLoc.mosLongitude),
                         beans::PropertyState_DIRECT_VALUE });
    if (rLoc.mosEntityName.has_value())
        aVec.push_back({ u"entityName"_ustr, 0, uno::Any(*rLoc.mosEntityName),
                         beans::PropertyState_DIRECT_VALUE });
    if (rLoc.mosEntityType.has_value())
        aVec.push_back({ u"entityType"_ustr, 0, uno::Any(*rLoc.mosEntityType),
                         beans::PropertyState_DIRECT_VALUE });
    if (rLoc.mxAddress.has_value())
        aVec.push_back({ u"address"_ustr, 0, uno::Any(addressToProps(*rLoc.mxAddress)),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoLocationModel locationFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoLocationModel aLoc;
    for (const auto& rProp : rProps)
    {
        if (rProp.Name == "latitude")
        {
            OUString s;
            if (rProp.Value >>= s)
                aLoc.mosLatitude = s;
        }
        else if (rProp.Name == "longitude")
        {
            OUString s;
            if (rProp.Value >>= s)
                aLoc.mosLongitude = s;
        }
        else if (rProp.Name == "entityName")
        {
            OUString s;
            if (rProp.Value >>= s)
                aLoc.mosEntityName = s;
        }
        else if (rProp.Name == "entityType")
        {
            OUString s;
            if (rProp.Value >>= s)
                aLoc.mosEntityType = s;
        }
        else if (rProp.Name == "address")
        {
            uno::Sequence<beans::PropertyValue> aInner;
            if (rProp.Value >>= aInner)
                aLoc.mxAddress = addressFromProps(aInner);
        }
    }
    return aLoc;
}

uno::Sequence<beans::PropertyValue> locQueryToProps(const GeoLocationQueryModel& rQ)
{
    std::vector<beans::PropertyValue> aVec;
    if (rQ.mosCountryRegion.has_value())
        aVec.push_back({ u"countryRegion"_ustr, 0, uno::Any(*rQ.mosCountryRegion),
                         beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosAdminDistrict1.has_value())
        aVec.push_back({ u"adminDistrict1"_ustr, 0, uno::Any(*rQ.mosAdminDistrict1),
                         beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosAdminDistrict2.has_value())
        aVec.push_back({ u"adminDistrict2"_ustr, 0, uno::Any(*rQ.mosAdminDistrict2),
                         beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosPostalCode.has_value())
        aVec.push_back({ u"postalCode"_ustr, 0, uno::Any(*rQ.mosPostalCode),
                         beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosEntityType.has_value())
        aVec.push_back({ u"entityType"_ustr, 0, uno::Any(*rQ.mosEntityType),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoLocationQueryModel locQueryFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoLocationQueryModel aQ;
    for (const auto& rProp : rProps)
    {
        OUString s;
        if (!(rProp.Value >>= s))
            continue;
        if (rProp.Name == "countryRegion")
            aQ.mosCountryRegion = s;
        else if (rProp.Name == "adminDistrict1")
            aQ.mosAdminDistrict1 = s;
        else if (rProp.Name == "adminDistrict2")
            aQ.mosAdminDistrict2 = s;
        else if (rProp.Name == "postalCode")
            aQ.mosPostalCode = s;
        else if (rProp.Name == "entityType")
            aQ.mosEntityType = s;
    }
    return aQ;
}

uno::Sequence<beans::PropertyValue> locResultToProps(const GeoLocationQueryResultModel& rR)
{
    std::vector<beans::PropertyValue> aVec;
    if (rR.mxQuery.has_value())
        aVec.push_back({ u"query"_ustr, 0, uno::Any(locQueryToProps(*rR.mxQuery)),
                         beans::PropertyState_DIRECT_VALUE });
    std::vector<uno::Sequence<beans::PropertyValue>> aLocs;
    for (const auto& rLoc : rR.maLocations)
        aLocs.push_back(locationToProps(rLoc));
    if (!aLocs.empty())
        aVec.push_back({ u"locations"_ustr, 0, uno::Any(comphelper::containerToSequence(aLocs)),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoLocationQueryResultModel locResultFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoLocationQueryResultModel aR;
    for (const auto& rProp : rProps)
    {
        if (rProp.Name == "query")
        {
            uno::Sequence<beans::PropertyValue> aInner;
            if (rProp.Value >>= aInner)
                aR.mxQuery = locQueryFromProps(aInner);
        }
        else if (rProp.Name == "locations")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rL : aArr)
                    aR.maLocations.push_back(locationFromProps(rL));
        }
    }
    return aR;
}

uno::Sequence<beans::PropertyValue> polygonToProps(const GeoPolygonModel& rP)
{
    std::vector<beans::PropertyValue> aVec;
    if (rP.mosPolygonId.has_value())
        aVec.push_back({ u"polygonId"_ustr, 0, uno::Any(*rP.mosPolygonId),
                         beans::PropertyState_DIRECT_VALUE });
    if (rP.mosNumPoints.has_value())
        aVec.push_back({ u"numPoints"_ustr, 0, uno::Any(*rP.mosNumPoints),
                         beans::PropertyState_DIRECT_VALUE });
    if (rP.mosPcaRings.has_value())
        aVec.push_back(
            { u"pcaRings"_ustr, 0, uno::Any(*rP.mosPcaRings), beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoPolygonModel polygonFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoPolygonModel aP;
    for (const auto& rProp : rProps)
    {
        OUString s;
        if (!(rProp.Value >>= s))
            continue;
        if (rProp.Name == "polygonId")
            aP.mosPolygonId = s;
        else if (rProp.Name == "numPoints")
            aP.mosNumPoints = s;
        else if (rProp.Name == "pcaRings")
            aP.mosPcaRings = s;
    }
    return aP;
}

uno::Sequence<beans::PropertyValue> geoDataToProps(const GeoDataModel& rD)
{
    std::vector<beans::PropertyValue> aVec;
    if (rD.mosEntityName.has_value())
        aVec.push_back({ u"entityName"_ustr, 0, uno::Any(*rD.mosEntityName),
                         beans::PropertyState_DIRECT_VALUE });
    if (rD.mosEntityId.has_value())
        aVec.push_back(
            { u"entityId"_ustr, 0, uno::Any(*rD.mosEntityId), beans::PropertyState_DIRECT_VALUE });
    if (rD.mosEast.has_value())
        aVec.push_back(
            { u"east"_ustr, 0, uno::Any(*rD.mosEast), beans::PropertyState_DIRECT_VALUE });
    if (rD.mosWest.has_value())
        aVec.push_back(
            { u"west"_ustr, 0, uno::Any(*rD.mosWest), beans::PropertyState_DIRECT_VALUE });
    if (rD.mosNorth.has_value())
        aVec.push_back(
            { u"north"_ustr, 0, uno::Any(*rD.mosNorth), beans::PropertyState_DIRECT_VALUE });
    if (rD.mosSouth.has_value())
        aVec.push_back(
            { u"south"_ustr, 0, uno::Any(*rD.mosSouth), beans::PropertyState_DIRECT_VALUE });
    if (!rD.maPolygons.empty())
    {
        std::vector<uno::Sequence<beans::PropertyValue>> aPolys;
        for (const auto& rP : rD.maPolygons)
            aPolys.push_back(polygonToProps(rP));
        aVec.push_back({ u"polygons"_ustr, 0, uno::Any(comphelper::containerToSequence(aPolys)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    if (!rD.maCopyrights.empty())
    {
        std::vector<OUString> aCR;
        for (const auto& rC : rD.maCopyrights)
            aCR.push_back(rC.msText);
        aVec.push_back({ u"copyrights"_ustr, 0, uno::Any(comphelper::containerToSequence(aCR)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    return comphelper::containerToSequence(aVec);
}

GeoDataModel geoDataFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoDataModel aD;
    for (const auto& rProp : rProps)
    {
        if (rProp.Name == "polygons")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rP : aArr)
                    aD.maPolygons.push_back(polygonFromProps(rP));
        }
        else if (rProp.Name == "copyrights")
        {
            uno::Sequence<OUString> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rC : aArr)
                    aD.maCopyrights.push_back({ rC });
        }
        else
        {
            OUString s;
            if (!(rProp.Value >>= s))
                continue;
            if (rProp.Name == "entityName")
                aD.mosEntityName = s;
            else if (rProp.Name == "entityId")
                aD.mosEntityId = s;
            else if (rProp.Name == "east")
                aD.mosEast = s;
            else if (rProp.Name == "west")
                aD.mosWest = s;
            else if (rProp.Name == "north")
                aD.mosNorth = s;
            else if (rProp.Name == "south")
                aD.mosSouth = s;
        }
    }
    return aD;
}

uno::Sequence<beans::PropertyValue> dataEntityQueryToProps(const GeoDataEntityQueryModel& rQ)
{
    std::vector<beans::PropertyValue> aVec;
    if (rQ.mosEntityType.has_value())
        aVec.push_back({ u"entityType"_ustr, 0, uno::Any(*rQ.mosEntityType),
                         beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosEntityId.has_value())
        aVec.push_back(
            { u"entityId"_ustr, 0, uno::Any(*rQ.mosEntityId), beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoDataEntityQueryModel dataEntityQueryFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoDataEntityQueryModel aQ;
    for (const auto& rProp : rProps)
    {
        OUString s;
        if (!(rProp.Value >>= s))
            continue;
        if (rProp.Name == "entityType")
            aQ.mosEntityType = s;
        else if (rProp.Name == "entityId")
            aQ.mosEntityId = s;
    }
    return aQ;
}

uno::Sequence<beans::PropertyValue> dataEntityResultToProps(const GeoDataEntityQueryResultModel& rR)
{
    std::vector<beans::PropertyValue> aVec;
    if (rR.mxQuery.has_value())
        aVec.push_back({ u"query"_ustr, 0, uno::Any(dataEntityQueryToProps(*rR.mxQuery)),
                         beans::PropertyState_DIRECT_VALUE });
    if (rR.mxData.has_value())
        aVec.push_back({ u"data"_ustr, 0, uno::Any(geoDataToProps(*rR.mxData)),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoDataEntityQueryResultModel
dataEntityResultFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoDataEntityQueryResultModel aR;
    for (const auto& rProp : rProps)
    {
        uno::Sequence<beans::PropertyValue> aInner;
        if (!(rProp.Value >>= aInner))
            continue;
        if (rProp.Name == "query")
            aR.mxQuery = dataEntityQueryFromProps(aInner);
        else if (rProp.Name == "data")
            aR.mxData = geoDataFromProps(aInner);
    }
    return aR;
}

uno::Sequence<beans::PropertyValue> ptQueryToProps(const GeoDataPointQueryModel& rQ)
{
    std::vector<beans::PropertyValue> aVec;
    if (rQ.mosEntityType.has_value())
        aVec.push_back({ u"entityType"_ustr, 0, uno::Any(*rQ.mosEntityType),
                         beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosLatitude.has_value())
        aVec.push_back(
            { u"latitude"_ustr, 0, uno::Any(*rQ.mosLatitude), beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosLongitude.has_value())
        aVec.push_back({ u"longitude"_ustr, 0, uno::Any(*rQ.mosLongitude),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoDataPointQueryModel ptQueryFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoDataPointQueryModel aQ;
    for (const auto& rProp : rProps)
    {
        OUString s;
        if (!(rProp.Value >>= s))
            continue;
        if (rProp.Name == "entityType")
            aQ.mosEntityType = s;
        else if (rProp.Name == "latitude")
            aQ.mosLatitude = s;
        else if (rProp.Name == "longitude")
            aQ.mosLongitude = s;
    }
    return aQ;
}

uno::Sequence<beans::PropertyValue> ptEntityQueryToProps(const GeoDataPointToEntityQueryModel& rQ)
{
    std::vector<beans::PropertyValue> aVec;
    if (rQ.mosEntityType.has_value())
        aVec.push_back({ u"entityType"_ustr, 0, uno::Any(*rQ.mosEntityType),
                         beans::PropertyState_DIRECT_VALUE });
    if (rQ.mosEntityId.has_value())
        aVec.push_back(
            { u"entityId"_ustr, 0, uno::Any(*rQ.mosEntityId), beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoDataPointToEntityQueryModel
ptEntityQueryFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoDataPointToEntityQueryModel aQ;
    for (const auto& rProp : rProps)
    {
        OUString s;
        if (!(rProp.Value >>= s))
            continue;
        if (rProp.Name == "entityType")
            aQ.mosEntityType = s;
        else if (rProp.Name == "entityId")
            aQ.mosEntityId = s;
    }
    return aQ;
}

uno::Sequence<beans::PropertyValue> ptResultToProps(const GeoDataPointToEntityQueryResultModel& rR)
{
    std::vector<beans::PropertyValue> aVec;
    if (rR.mxPointQuery.has_value())
        aVec.push_back({ u"pointQuery"_ustr, 0, uno::Any(ptQueryToProps(*rR.mxPointQuery)),
                         beans::PropertyState_DIRECT_VALUE });
    if (rR.mxEntityQuery.has_value())
        aVec.push_back({ u"entityQuery"_ustr, 0, uno::Any(ptEntityQueryToProps(*rR.mxEntityQuery)),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoDataPointToEntityQueryResultModel
ptResultFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoDataPointToEntityQueryResultModel aR;
    for (const auto& rProp : rProps)
    {
        uno::Sequence<beans::PropertyValue> aInner;
        if (!(rProp.Value >>= aInner))
            continue;
        if (rProp.Name == "pointQuery")
            aR.mxPointQuery = ptQueryFromProps(aInner);
        else if (rProp.Name == "entityQuery")
            aR.mxEntityQuery = ptEntityQueryFromProps(aInner);
    }
    return aR;
}

uno::Sequence<beans::PropertyValue> childQueryToProps(const GeoChildEntitiesQueryModel& rQ)
{
    std::vector<beans::PropertyValue> aVec;
    if (rQ.mosEntityId.has_value())
        aVec.push_back(
            { u"entityId"_ustr, 0, uno::Any(*rQ.mosEntityId), beans::PropertyState_DIRECT_VALUE });
    if (!rQ.maEntityTypes.empty())
        aVec.push_back({ u"entityTypes"_ustr, 0,
                         uno::Any(comphelper::containerToSequence(rQ.maEntityTypes)),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoChildEntitiesQueryModel childQueryFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoChildEntitiesQueryModel aQ;
    for (const auto& rProp : rProps)
    {
        if (rProp.Name == "entityId")
        {
            OUString s;
            if (rProp.Value >>= s)
                aQ.mosEntityId = s;
        }
        else if (rProp.Name == "entityTypes")
        {
            uno::Sequence<OUString> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rS : aArr)
                    aQ.maEntityTypes.push_back(rS);
        }
    }
    return aQ;
}

uno::Sequence<beans::PropertyValue> hierarchyEntityToProps(const GeoHierarchyEntityModel& rE)
{
    std::vector<beans::PropertyValue> aVec;
    if (rE.mosEntityName.has_value())
        aVec.push_back({ u"entityName"_ustr, 0, uno::Any(*rE.mosEntityName),
                         beans::PropertyState_DIRECT_VALUE });
    if (rE.mosEntityId.has_value())
        aVec.push_back(
            { u"entityId"_ustr, 0, uno::Any(*rE.mosEntityId), beans::PropertyState_DIRECT_VALUE });
    if (rE.mosEntityType.has_value())
        aVec.push_back({ u"entityType"_ustr, 0, uno::Any(*rE.mosEntityType),
                         beans::PropertyState_DIRECT_VALUE });
    return comphelper::containerToSequence(aVec);
}

GeoHierarchyEntityModel hierarchyEntityFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoHierarchyEntityModel aE;
    for (const auto& rProp : rProps)
    {
        OUString s;
        if (!(rProp.Value >>= s))
            continue;
        if (rProp.Name == "entityName")
            aE.mosEntityName = s;
        else if (rProp.Name == "entityId")
            aE.mosEntityId = s;
        else if (rProp.Name == "entityType")
            aE.mosEntityType = s;
    }
    return aE;
}

uno::Sequence<beans::PropertyValue> childResultToProps(const GeoChildEntitiesQueryResultModel& rR)
{
    std::vector<beans::PropertyValue> aVec;
    if (rR.mxQuery.has_value())
        aVec.push_back({ u"query"_ustr, 0, uno::Any(childQueryToProps(*rR.mxQuery)),
                         beans::PropertyState_DIRECT_VALUE });
    if (!rR.maEntities.empty())
    {
        std::vector<uno::Sequence<beans::PropertyValue>> aArr;
        for (const auto& rE : rR.maEntities)
            aArr.push_back(hierarchyEntityToProps(rE));
        aVec.push_back({ u"entities"_ustr, 0, uno::Any(comphelper::containerToSequence(aArr)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    return comphelper::containerToSequence(aVec);
}

GeoChildEntitiesQueryResultModel
childResultFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoChildEntitiesQueryResultModel aR;
    for (const auto& rProp : rProps)
    {
        if (rProp.Name == "query")
        {
            uno::Sequence<beans::PropertyValue> aInner;
            if (rProp.Value >>= aInner)
                aR.mxQuery = childQueryFromProps(aInner);
        }
        else if (rProp.Name == "entities")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rE : aArr)
                    aR.maEntities.push_back(hierarchyEntityFromProps(rE));
        }
    }
    return aR;
}

uno::Sequence<beans::PropertyValue> parentResultToProps(const GeoParentEntitiesQueryResultModel& rR)
{
    std::vector<beans::PropertyValue> aVec;
    if (rR.mxQuery.has_value())
    {
        std::vector<beans::PropertyValue> aQ;
        if (rR.mxQuery->mosEntityId.has_value())
            aQ.push_back({ u"entityId"_ustr, 0, uno::Any(*rR.mxQuery->mosEntityId),
                           beans::PropertyState_DIRECT_VALUE });
        aVec.push_back({ u"query"_ustr, 0, uno::Any(comphelper::containerToSequence(aQ)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    if (rR.mxEntity.has_value())
    {
        std::vector<beans::PropertyValue> aE;
        if (rR.mxEntity->mosEntityName.has_value())
            aE.push_back({ u"entityName"_ustr, 0, uno::Any(*rR.mxEntity->mosEntityName),
                           beans::PropertyState_DIRECT_VALUE });
        if (rR.mxEntity->mosEntityType.has_value())
            aE.push_back({ u"entityType"_ustr, 0, uno::Any(*rR.mxEntity->mosEntityType),
                           beans::PropertyState_DIRECT_VALUE });
        aVec.push_back({ u"entity"_ustr, 0, uno::Any(comphelper::containerToSequence(aE)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    if (rR.mxParentEntity.has_value())
    {
        std::vector<beans::PropertyValue> aP;
        if (rR.mxParentEntity->mosEntityId.has_value())
            aP.push_back({ u"entityId"_ustr, 0, uno::Any(*rR.mxParentEntity->mosEntityId),
                           beans::PropertyState_DIRECT_VALUE });
        aVec.push_back({ u"parentEntity"_ustr, 0, uno::Any(comphelper::containerToSequence(aP)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    return comphelper::containerToSequence(aVec);
}

GeoParentEntitiesQueryResultModel
parentResultFromProps(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoParentEntitiesQueryResultModel aR;
    for (const auto& rProp : rProps)
    {
        uno::Sequence<beans::PropertyValue> aInner;
        if (!(rProp.Value >>= aInner))
            continue;
        if (rProp.Name == "query")
        {
            aR.mxQuery.emplace();
            for (const auto& rQ : aInner)
            {
                OUString s;
                if (rQ.Name == "entityId" && (rQ.Value >>= s))
                    aR.mxQuery->mosEntityId = s;
            }
        }
        else if (rProp.Name == "entity")
        {
            aR.mxEntity.emplace();
            for (const auto& rE : aInner)
            {
                OUString s;
                if (!(rE.Value >>= s))
                    continue;
                if (rE.Name == "entityName")
                    aR.mxEntity->mosEntityName = s;
                else if (rE.Name == "entityType")
                    aR.mxEntity->mosEntityType = s;
            }
        }
        else if (rProp.Name == "parentEntity")
        {
            aR.mxParentEntity.emplace();
            for (const auto& rP : aInner)
            {
                OUString s;
                if (rP.Name == "entityId" && (rP.Value >>= s))
                    aR.mxParentEntity->mosEntityId = s;
            }
        }
    }
    return aR;
}

} // anonymous namespace

uno::Sequence<beans::PropertyValue> geoClearToPropertyValues(const GeoClearModel& rModel)
{
    std::vector<beans::PropertyValue> aVec;

    if (!rModel.maLocResults.empty())
    {
        std::vector<uno::Sequence<beans::PropertyValue>> aArr;
        for (const auto& rR : rModel.maLocResults)
            aArr.push_back(locResultToProps(rR));
        aVec.push_back({ u"locResults"_ustr, 0, uno::Any(comphelper::containerToSequence(aArr)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    if (!rModel.maDataResults.empty())
    {
        std::vector<uno::Sequence<beans::PropertyValue>> aArr;
        for (const auto& rR : rModel.maDataResults)
            aArr.push_back(dataEntityResultToProps(rR));
        aVec.push_back({ u"dataResults"_ustr, 0, uno::Any(comphelper::containerToSequence(aArr)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    if (!rModel.maPtResults.empty())
    {
        std::vector<uno::Sequence<beans::PropertyValue>> aArr;
        for (const auto& rR : rModel.maPtResults)
            aArr.push_back(ptResultToProps(rR));
        aVec.push_back({ u"ptResults"_ustr, 0, uno::Any(comphelper::containerToSequence(aArr)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    if (!rModel.maChildResults.empty())
    {
        std::vector<uno::Sequence<beans::PropertyValue>> aArr;
        for (const auto& rR : rModel.maChildResults)
            aArr.push_back(childResultToProps(rR));
        aVec.push_back({ u"childResults"_ustr, 0, uno::Any(comphelper::containerToSequence(aArr)),
                         beans::PropertyState_DIRECT_VALUE });
    }
    if (!rModel.maParentResults.empty())
    {
        std::vector<uno::Sequence<beans::PropertyValue>> aArr;
        for (const auto& rR : rModel.maParentResults)
            aArr.push_back(parentResultToProps(rR));
        aVec.push_back({ u"parentResults"_ustr, 0, uno::Any(comphelper::containerToSequence(aArr)),
                         beans::PropertyState_DIRECT_VALUE });
    }

    return comphelper::containerToSequence(aVec);
}

GeoClearModel geoClearFromPropertyValues(const uno::Sequence<beans::PropertyValue>& rProps)
{
    GeoClearModel aModel;
    for (const auto& rProp : rProps)
    {
        if (rProp.Name == "locResults")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rR : aArr)
                    aModel.maLocResults.push_back(locResultFromProps(rR));
        }
        else if (rProp.Name == "dataResults")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rR : aArr)
                    aModel.maDataResults.push_back(dataEntityResultFromProps(rR));
        }
        else if (rProp.Name == "ptResults")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rR : aArr)
                    aModel.maPtResults.push_back(ptResultFromProps(rR));
        }
        else if (rProp.Name == "childResults")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rR : aArr)
                    aModel.maChildResults.push_back(childResultFromProps(rR));
        }
        else if (rProp.Name == "parentResults")
        {
            uno::Sequence<uno::Sequence<beans::PropertyValue>> aArr;
            if (rProp.Value >>= aArr)
                for (const auto& rR : aArr)
                    aModel.maParentResults.push_back(parentResultFromProps(rR));
        }
    }
    return aModel;
}

void convertGeography(const GeographyModel& rModel, PropertySet& rSeriesProp)
{
    if (rModel.mosProjectionType.has_value())
        rSeriesProp.setProperty(PROP_GeographyProjectionType, *rModel.mosProjectionType);
    if (rModel.mosViewedRegionType.has_value())
        rSeriesProp.setProperty(PROP_GeographyViewedRegionType, *rModel.mosViewedRegionType);
    if (rModel.mosCultureLanguage.has_value())
        rSeriesProp.setProperty(PROP_GeographyCultureLanguage, *rModel.mosCultureLanguage);
    if (rModel.mosCultureRegion.has_value())
        rSeriesProp.setProperty(PROP_GeographyCultureRegion, *rModel.mosCultureRegion);
    if (rModel.mosAttribution.has_value())
        rSeriesProp.setProperty(PROP_GeographyAttribution, *rModel.mosAttribution);

    if (rModel.mxGeoCache.has_value())
    {
        const auto& rCache = *rModel.mxGeoCache;
        if (rCache.mosProvider.has_value())
            rSeriesProp.setProperty(PROP_GeoCacheProvider, *rCache.mosProvider);
        if (rCache.mosBinary.has_value())
            rSeriesProp.setProperty(PROP_GeoCacheBinary, *rCache.mosBinary);
        if (rCache.mxClear.has_value())
            rSeriesProp.setProperty(PROP_GeoCacheClearData,
                                    geoClearToPropertyValues(*rCache.mxClear));
    }
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
