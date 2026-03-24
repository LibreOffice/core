/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/export/chartgeographyexport.hxx>
#include <drawingml/chart/geographyconverter.hxx>
#include <drawingml/chart/geographymodel.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

// Code for exporting <cx:geography> content in a chartex chart, used for the
// regionMap chart type.

using namespace ::com::sun::star;

namespace oox::drawingml::chart
{
namespace
{
const char* toNullableStr(const std::optional<OUString>& rOpt, OString& rBuf)
{
    if (!rOpt.has_value())
        return nullptr;
    rBuf = rOpt->toUtf8();
    return rBuf.getStr();
}

void writeAddress(const GeoAddressModel& rAddr, const sax_fastparser::FSHelperPtr& pFS)
{
    OString b1, b2, b3, b4, b5, b6, b7;
    pFS->singleElement(
        FSNS(XML_cx, XML_address), XML_address1, toNullableStr(rAddr.mosAddress1, b1),
        XML_countryRegion, toNullableStr(rAddr.mosCountryRegion, b2), XML_adminDistrict1,
        toNullableStr(rAddr.mosAdminDistrict1, b3), XML_adminDistrict2,
        toNullableStr(rAddr.mosAdminDistrict2, b4), XML_postalCode,
        toNullableStr(rAddr.mosPostalCode, b5), XML_locality, toNullableStr(rAddr.mosLocality, b6),
        XML_isoCountryCode, toNullableStr(rAddr.mosIsoCountryCode, b7));
}

void writeLocResults(const std::vector<GeoLocationQueryResultModel>& rResults,
                     const sax_fastparser::FSHelperPtr& pFS)
{
    if (rResults.empty())
        return;
    pFS->startElement(FSNS(XML_cx, XML_geoLocationQueryResults));
    for (const auto& rR : rResults)
    {
        pFS->startElement(FSNS(XML_cx, XML_geoLocationQueryResult));
        if (rR.mxQuery.has_value())
        {
            const auto& q = *rR.mxQuery;
            OString b1, b2, b3, b4, b5;
            pFS->singleElement(FSNS(XML_cx, XML_geoLocationQuery), XML_countryRegion,
                               toNullableStr(q.mosCountryRegion, b1), XML_adminDistrict1,
                               toNullableStr(q.mosAdminDistrict1, b2), XML_adminDistrict2,
                               toNullableStr(q.mosAdminDistrict2, b3), XML_postalCode,
                               toNullableStr(q.mosPostalCode, b4), XML_entityType,
                               toNullableStr(q.mosEntityType, b5));
        }
        if (!rR.maLocations.empty())
        {
            pFS->startElement(FSNS(XML_cx, XML_geoLocations));
            for (const auto& rLoc : rR.maLocations)
            {
                OString b1, b2, b3, b4;
                if (rLoc.mxAddress.has_value())
                {
                    pFS->startElement(FSNS(XML_cx, XML_geoLocation), XML_latitude,
                                      toNullableStr(rLoc.mosLatitude, b1), XML_longitude,
                                      toNullableStr(rLoc.mosLongitude, b2), XML_entityName,
                                      toNullableStr(rLoc.mosEntityName, b3), XML_entityType,
                                      toNullableStr(rLoc.mosEntityType, b4));
                    writeAddress(*rLoc.mxAddress, pFS);
                    pFS->endElement(FSNS(XML_cx, XML_geoLocation));
                }
                else
                {
                    pFS->singleElement(FSNS(XML_cx, XML_geoLocation), XML_latitude,
                                       toNullableStr(rLoc.mosLatitude, b1), XML_longitude,
                                       toNullableStr(rLoc.mosLongitude, b2), XML_entityName,
                                       toNullableStr(rLoc.mosEntityName, b3), XML_entityType,
                                       toNullableStr(rLoc.mosEntityType, b4));
                }
            }
            pFS->endElement(FSNS(XML_cx, XML_geoLocations));
        }
        pFS->endElement(FSNS(XML_cx, XML_geoLocationQueryResult));
    }
    pFS->endElement(FSNS(XML_cx, XML_geoLocationQueryResults));
}

void writeDataResults(const std::vector<GeoDataEntityQueryResultModel>& rResults,
                      const sax_fastparser::FSHelperPtr& pFS)
{
    if (rResults.empty())
        return;
    pFS->startElement(FSNS(XML_cx, XML_geoDataEntityQueryResults));
    for (const auto& rR : rResults)
    {
        pFS->startElement(FSNS(XML_cx, XML_geoDataEntityQueryResult));
        if (rR.mxQuery.has_value())
        {
            OString b1, b2;
            pFS->singleElement(FSNS(XML_cx, XML_geoDataEntityQuery), XML_entityType,
                               toNullableStr(rR.mxQuery->mosEntityType, b1), XML_entityId,
                               toNullableStr(rR.mxQuery->mosEntityId, b2));
        }
        if (rR.mxData.has_value())
        {
            const auto& d = *rR.mxData;
            OString b1, b2, b3, b4, b5, b6;
            pFS->startElement(
                FSNS(XML_cx, XML_geoData), XML_entityName, toNullableStr(d.mosEntityName, b1),
                XML_entityId, toNullableStr(d.mosEntityId, b2), XML_east,
                toNullableStr(d.mosEast, b3), XML_west, toNullableStr(d.mosWest, b4), XML_north,
                toNullableStr(d.mosNorth, b5), XML_south, toNullableStr(d.mosSouth, b6));

            if (!d.maPolygons.empty())
            {
                pFS->startElement(FSNS(XML_cx, XML_geoPolygons));
                for (const auto& rP : d.maPolygons)
                {
                    OString pb1, pb2, pb3;
                    pFS->singleElement(FSNS(XML_cx, XML_geoPolygon), XML_polygonId,
                                       toNullableStr(rP.mosPolygonId, pb1), XML_numPoints,
                                       toNullableStr(rP.mosNumPoints, pb2), XML_pcaRings,
                                       toNullableStr(rP.mosPcaRings, pb3));
                }
                pFS->endElement(FSNS(XML_cx, XML_geoPolygons));
            }

            if (!d.maCopyrights.empty())
            {
                pFS->startElement(FSNS(XML_cx, XML_copyrights));
                for (const auto& rC : d.maCopyrights)
                {
                    pFS->startElement(FSNS(XML_cx, XML_copyright));
                    pFS->writeEscaped(rC.msText);
                    pFS->endElement(FSNS(XML_cx, XML_copyright));
                }
                pFS->endElement(FSNS(XML_cx, XML_copyrights));
            }

            pFS->endElement(FSNS(XML_cx, XML_geoData));
        }
        pFS->endElement(FSNS(XML_cx, XML_geoDataEntityQueryResult));
    }
    pFS->endElement(FSNS(XML_cx, XML_geoDataEntityQueryResults));
}

void writePtResults(const std::vector<GeoDataPointToEntityQueryResultModel>& rResults,
                    const sax_fastparser::FSHelperPtr& pFS)
{
    if (rResults.empty())
        return;
    pFS->startElement(FSNS(XML_cx, XML_geoDataPointToEntityQueryResults));
    for (const auto& rR : rResults)
    {
        pFS->startElement(FSNS(XML_cx, XML_geoDataPointToEntityQueryResult));
        if (rR.mxPointQuery.has_value())
        {
            OString b1, b2, b3;
            pFS->singleElement(FSNS(XML_cx, XML_geoDataPointQuery), XML_entityType,
                               toNullableStr(rR.mxPointQuery->mosEntityType, b1), XML_latitude,
                               toNullableStr(rR.mxPointQuery->mosLatitude, b2), XML_longitude,
                               toNullableStr(rR.mxPointQuery->mosLongitude, b3));
        }
        if (rR.mxEntityQuery.has_value())
        {
            OString b1, b2;
            pFS->singleElement(FSNS(XML_cx, XML_geoDataPointToEntityQuery), XML_entityType,
                               toNullableStr(rR.mxEntityQuery->mosEntityType, b1), XML_entityId,
                               toNullableStr(rR.mxEntityQuery->mosEntityId, b2));
        }
        pFS->endElement(FSNS(XML_cx, XML_geoDataPointToEntityQueryResult));
    }
    pFS->endElement(FSNS(XML_cx, XML_geoDataPointToEntityQueryResults));
}

void writeChildResults(const std::vector<GeoChildEntitiesQueryResultModel>& rResults,
                       const sax_fastparser::FSHelperPtr& pFS)
{
    if (rResults.empty())
        return;
    pFS->startElement(FSNS(XML_cx, XML_geoChildEntitiesQueryResults));
    for (const auto& rR : rResults)
    {
        pFS->startElement(FSNS(XML_cx, XML_geoChildEntitiesQueryResult));
        if (rR.mxQuery.has_value())
        {
            OString b1;
            pFS->startElement(FSNS(XML_cx, XML_geoChildEntitiesQuery), XML_entityId,
                              toNullableStr(rR.mxQuery->mosEntityId, b1));
            if (!rR.mxQuery->maEntityTypes.empty())
            {
                pFS->startElement(FSNS(XML_cx, XML_geoChildTypes));
                for (const auto& rT : rR.mxQuery->maEntityTypes)
                {
                    pFS->startElement(FSNS(XML_cx, XML_entityType));
                    pFS->writeEscaped(rT);
                    pFS->endElement(FSNS(XML_cx, XML_entityType));
                }
                pFS->endElement(FSNS(XML_cx, XML_geoChildTypes));
            }
            pFS->endElement(FSNS(XML_cx, XML_geoChildEntitiesQuery));
        }
        if (!rR.maEntities.empty())
        {
            pFS->startElement(FSNS(XML_cx, XML_geoChildEntities));
            for (const auto& rE : rR.maEntities)
            {
                OString b1, b2, b3;
                pFS->singleElement(FSNS(XML_cx, XML_geoHierarchyEntity), XML_entityName,
                                   toNullableStr(rE.mosEntityName, b1), XML_entityId,
                                   toNullableStr(rE.mosEntityId, b2), XML_entityType,
                                   toNullableStr(rE.mosEntityType, b3));
            }
            pFS->endElement(FSNS(XML_cx, XML_geoChildEntities));
        }
        pFS->endElement(FSNS(XML_cx, XML_geoChildEntitiesQueryResult));
    }
    pFS->endElement(FSNS(XML_cx, XML_geoChildEntitiesQueryResults));
}

void writeParentResults(const std::vector<GeoParentEntitiesQueryResultModel>& rResults,
                        const sax_fastparser::FSHelperPtr& pFS)
{
    if (rResults.empty())
        return;
    pFS->startElement(FSNS(XML_cx, XML_geoParentEntitiesQueryResults));
    for (const auto& rR : rResults)
    {
        pFS->startElement(FSNS(XML_cx, XML_geoParentEntitiesQueryResult));
        if (rR.mxQuery.has_value())
        {
            OString b1;
            pFS->singleElement(FSNS(XML_cx, XML_geoParentEntitiesQuery), XML_entityId,
                               toNullableStr(rR.mxQuery->mosEntityId, b1));
        }
        if (rR.mxEntity.has_value())
        {
            OString b1, b2;
            pFS->singleElement(FSNS(XML_cx, XML_geoEntity), XML_entityName,
                               toNullableStr(rR.mxEntity->mosEntityName, b1), XML_entityType,
                               toNullableStr(rR.mxEntity->mosEntityType, b2));
        }
        if (rR.mxParentEntity.has_value())
        {
            OString b1;
            pFS->singleElement(FSNS(XML_cx, XML_geoParentEntity), XML_entityId,
                               toNullableStr(rR.mxParentEntity->mosEntityId, b1));
        }
        pFS->endElement(FSNS(XML_cx, XML_geoParentEntitiesQueryResult));
    }
    pFS->endElement(FSNS(XML_cx, XML_geoParentEntitiesQueryResults));
}

} // anonymous namespace

void exportGeography(const uno::Reference<beans::XPropertySet>& xSeriesProp,
                     const sax_fastparser::FSHelperPtr& pFS)
{
    if (!xSeriesProp.is())
        return;

    OUString sProjectionType, sViewedRegionType, sCultureLanguage, sCultureRegion, sAttribution;
    uno::Any aPT = xSeriesProp->getPropertyValue(u"GeographyProjectionType"_ustr);
    uno::Any aVR = xSeriesProp->getPropertyValue(u"GeographyViewedRegionType"_ustr);
    uno::Any aCL = xSeriesProp->getPropertyValue(u"GeographyCultureLanguage"_ustr);
    uno::Any aCR = xSeriesProp->getPropertyValue(u"GeographyCultureRegion"_ustr);
    uno::Any aAT = xSeriesProp->getPropertyValue(u"GeographyAttribution"_ustr);

    bool bHasPT = (aPT >>= sProjectionType);
    bool bHasVR = (aVR >>= sViewedRegionType);
    bool bHasCL = (aCL >>= sCultureLanguage);
    bool bHasCR = (aCR >>= sCultureRegion);
    bool bHasAT = (aAT >>= sAttribution);

    if (!bHasPT && !bHasVR && !bHasCL && !bHasCR && !bHasAT)
        return;

    pFS->startElement(FSNS(XML_cx, XML_geography), XML_projectionType,
                      bHasPT ? sProjectionType.toUtf8().getStr() : nullptr, XML_viewedRegionType,
                      bHasVR ? sViewedRegionType.toUtf8().getStr() : nullptr, XML_cultureLanguage,
                      bHasCL ? sCultureLanguage.toUtf8().getStr() : nullptr, XML_cultureRegion,
                      bHasCR ? sCultureRegion.toUtf8().getStr() : nullptr, XML_attribution,
                      bHasAT ? sAttribution.toUtf8().getStr() : nullptr);

    // Handle the geoCache child element
    OUString sProvider;
    uno::Any aProvider = xSeriesProp->getPropertyValue(u"GeoCacheProvider"_ustr);
    bool bHasProvider = (aProvider >>= sProvider);

    OUString sBinary;
    uno::Any aBinary = xSeriesProp->getPropertyValue(u"GeoCacheBinary"_ustr);
    bool bHasBinary = (aBinary >>= sBinary);

    uno::Sequence<beans::PropertyValue> aClearData;
    uno::Any aClear = xSeriesProp->getPropertyValue(u"GeoCacheClearData"_ustr);
    bool bHasClear = (aClear >>= aClearData) && aClearData.hasElements();

    if (bHasProvider || bHasBinary || bHasClear)
    {
        pFS->startElement(FSNS(XML_cx, XML_geoCache), XML_provider,
                          bHasProvider ? sProvider.toUtf8().getStr() : nullptr);

        if (bHasBinary)
        {
            pFS->startElement(FSNS(XML_cx, XML_binary));
            pFS->writeEscaped(sBinary);
            pFS->endElement(FSNS(XML_cx, XML_binary));
        }
        else if (bHasClear)
        {
            GeoClearModel aClearModel = geoClearFromPropertyValues(aClearData);

            pFS->startElement(FSNS(XML_cx, XML_clear));
            writeLocResults(aClearModel.maLocResults, pFS);
            writeDataResults(aClearModel.maDataResults, pFS);
            writePtResults(aClearModel.maPtResults, pFS);
            writeChildResults(aClearModel.maChildResults, pFS);
            writeParentResults(aClearModel.maParentResults, pFS);
            pFS->endElement(FSNS(XML_cx, XML_clear));
        }

        pFS->endElement(FSNS(XML_cx, XML_geoCache));
    }

    pFS->endElement(FSNS(XML_cx, XML_geography));
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
