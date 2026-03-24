/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawingml/chart/geographycontext.hxx>
#include <drawingml/chart/geographymodel.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml::chart
{
using ::oox::core::ContextHandlerRef;

GeographyContext::GeographyContext(::oox::core::ContextHandler2Helper& rParent,
                                   GeographyModel& rModel)
    : ContextHandler2(rParent)
    , mrModel(rModel)
{
}

// Handle the CT_Geography complex type in the chartex schema
ContextHandlerRef GeographyContext::onCreateContext(sal_Int32 nElement,
                                                    const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case CX_TOKEN(geography):
            switch (nElement)
            {
                case CX_TOKEN(geoCache):
                    mrModel.mxGeoCache.emplace();
                    mrModel.mxGeoCache->mosProvider = rAttribs.getString(XML_provider);
                    return this;
            }
            break;

        case CX_TOKEN(geoCache):
            switch (nElement)
            {
                case CX_TOKEN(binary):
                    return this;
                case CX_TOKEN(clear):
                    mrModel.mxGeoCache->mxClear.emplace();
                    return this;
            }
            break;

        case CX_TOKEN(clear):
            switch (nElement)
            {
                case CX_TOKEN(geoLocationQueryResults):
                case CX_TOKEN(geoDataEntityQueryResults):
                case CX_TOKEN(geoDataPointToEntityQueryResults):
                case CX_TOKEN(geoChildEntitiesQueryResults):
                case CX_TOKEN(geoParentEntitiesQueryResults):
                    return this;
            }
            break;

        case CX_TOKEN(geoLocationQueryResults):
            if (nElement == CX_TOKEN(geoLocationQueryResult))
            {
                auto& rClear = *mrModel.mxGeoCache->mxClear;
                rClear.maLocResults.emplace_back();
                mpCurLocResult = &rClear.maLocResults.back();
                return this;
            }
            break;

        case CX_TOKEN(geoLocationQueryResult):
            switch (nElement)
            {
                case CX_TOKEN(geoLocationQuery):
                {
                    mpCurLocResult->mxQuery.emplace();
                    auto& q = *mpCurLocResult->mxQuery;
                    q.mosCountryRegion = rAttribs.getString(XML_countryRegion);
                    q.mosAdminDistrict1 = rAttribs.getString(XML_adminDistrict1);
                    q.mosAdminDistrict2 = rAttribs.getString(XML_adminDistrict2);
                    q.mosPostalCode = rAttribs.getString(XML_postalCode);
                    q.mosEntityType = rAttribs.getString(XML_entityType);
                    return nullptr;
                }
                case CX_TOKEN(geoLocations):
                    return this;
            }
            break;

        case CX_TOKEN(geoLocations):
            if (nElement == CX_TOKEN(geoLocation))
            {
                mpCurLocResult->maLocations.emplace_back();
                auto& loc = mpCurLocResult->maLocations.back();
                loc.mosLatitude = rAttribs.getString(XML_latitude);
                loc.mosLongitude = rAttribs.getString(XML_longitude);
                loc.mosEntityName = rAttribs.getString(XML_entityName);
                loc.mosEntityType = rAttribs.getString(XML_entityType);
                return this;
            }
            break;

        case CX_TOKEN(geoLocation):
            if (nElement == CX_TOKEN(address))
            {
                auto& loc = mpCurLocResult->maLocations.back();
                loc.mxAddress.emplace();
                auto& a = *loc.mxAddress;
                a.mosAddress1 = rAttribs.getString(XML_address1);
                a.mosCountryRegion = rAttribs.getString(XML_countryRegion);
                a.mosAdminDistrict1 = rAttribs.getString(XML_adminDistrict1);
                a.mosAdminDistrict2 = rAttribs.getString(XML_adminDistrict2);
                a.mosPostalCode = rAttribs.getString(XML_postalCode);
                a.mosLocality = rAttribs.getString(XML_locality);
                a.mosIsoCountryCode = rAttribs.getString(XML_isoCountryCode);
                return nullptr;
            }
            break;

        case CX_TOKEN(geoDataEntityQueryResults):
            if (nElement == CX_TOKEN(geoDataEntityQueryResult))
            {
                auto& rClear = *mrModel.mxGeoCache->mxClear;
                rClear.maDataResults.emplace_back();
                mpCurDataResult = &rClear.maDataResults.back();
                return this;
            }
            break;

        case CX_TOKEN(geoDataEntityQueryResult):
            switch (nElement)
            {
                case CX_TOKEN(geoDataEntityQuery):
                {
                    mpCurDataResult->mxQuery.emplace();
                    auto& q = *mpCurDataResult->mxQuery;
                    q.mosEntityType = rAttribs.getString(XML_entityType);
                    q.mosEntityId = rAttribs.getString(XML_entityId);
                    return nullptr;
                }
                case CX_TOKEN(geoData):
                {
                    mpCurDataResult->mxData.emplace();
                    auto& d = *mpCurDataResult->mxData;
                    d.mosEntityName = rAttribs.getString(XML_entityName);
                    d.mosEntityId = rAttribs.getString(XML_entityId);
                    d.mosEast = rAttribs.getString(XML_east);
                    d.mosWest = rAttribs.getString(XML_west);
                    d.mosNorth = rAttribs.getString(XML_north);
                    d.mosSouth = rAttribs.getString(XML_south);
                    return this;
                }
            }
            break;

        case CX_TOKEN(geoData):
            switch (nElement)
            {
                case CX_TOKEN(geoPolygons):
                    return this;
                case CX_TOKEN(copyrights):
                    return this;
            }
            break;

        case CX_TOKEN(geoPolygons):
            if (nElement == CX_TOKEN(geoPolygon))
            {
                auto& d = *mpCurDataResult->mxData;
                d.maPolygons.emplace_back();
                auto& p = d.maPolygons.back();
                p.mosPolygonId = rAttribs.getString(XML_polygonId);
                p.mosNumPoints = rAttribs.getString(XML_numPoints);
                p.mosPcaRings = rAttribs.getString(XML_pcaRings);
                return nullptr;
            }
            break;

        case CX_TOKEN(copyrights):
            if (nElement == CX_TOKEN(copyright))
                return this;
            break;

        case CX_TOKEN(geoDataPointToEntityQueryResults):
            if (nElement == CX_TOKEN(geoDataPointToEntityQueryResult))
            {
                auto& rClear = *mrModel.mxGeoCache->mxClear;
                rClear.maPtResults.emplace_back();
                mpCurPtResult = &rClear.maPtResults.back();
                return this;
            }
            break;

        case CX_TOKEN(geoDataPointToEntityQueryResult):
            switch (nElement)
            {
                case CX_TOKEN(geoDataPointQuery):
                {
                    mpCurPtResult->mxPointQuery.emplace();
                    auto& q = *mpCurPtResult->mxPointQuery;
                    q.mosEntityType = rAttribs.getString(XML_entityType);
                    q.mosLatitude = rAttribs.getString(XML_latitude);
                    q.mosLongitude = rAttribs.getString(XML_longitude);
                    return nullptr;
                }
                case CX_TOKEN(geoDataPointToEntityQuery):
                {
                    mpCurPtResult->mxEntityQuery.emplace();
                    auto& q = *mpCurPtResult->mxEntityQuery;
                    q.mosEntityType = rAttribs.getString(XML_entityType);
                    q.mosEntityId = rAttribs.getString(XML_entityId);
                    return nullptr;
                }
            }
            break;

        case CX_TOKEN(geoChildEntitiesQueryResults):
            if (nElement == CX_TOKEN(geoChildEntitiesQueryResult))
            {
                auto& rClear = *mrModel.mxGeoCache->mxClear;
                rClear.maChildResults.emplace_back();
                mpCurChildResult = &rClear.maChildResults.back();
                return this;
            }
            break;

        case CX_TOKEN(geoChildEntitiesQueryResult):
            switch (nElement)
            {
                case CX_TOKEN(geoChildEntitiesQuery):
                {
                    mpCurChildResult->mxQuery.emplace();
                    mpCurChildResult->mxQuery->mosEntityId = rAttribs.getString(XML_entityId);
                    return this;
                }
                case CX_TOKEN(geoChildEntities):
                    return this;
            }
            break;

        case CX_TOKEN(geoChildEntitiesQuery):
            if (nElement == CX_TOKEN(geoChildTypes))
                return this;
            break;

        case CX_TOKEN(geoChildTypes):
            if (nElement == CX_TOKEN(entityType))
                return this;
            break;

        case CX_TOKEN(geoChildEntities):
            if (nElement == CX_TOKEN(geoHierarchyEntity))
            {
                mpCurChildResult->maEntities.emplace_back();
                auto& e = mpCurChildResult->maEntities.back();
                e.mosEntityName = rAttribs.getString(XML_entityName);
                e.mosEntityId = rAttribs.getString(XML_entityId);
                e.mosEntityType = rAttribs.getString(XML_entityType);
                return nullptr;
            }
            break;

        case CX_TOKEN(geoParentEntitiesQueryResults):
            if (nElement == CX_TOKEN(geoParentEntitiesQueryResult))
            {
                auto& rClear = *mrModel.mxGeoCache->mxClear;
                rClear.maParentResults.emplace_back();
                mpCurParentResult = &rClear.maParentResults.back();
                return this;
            }
            break;

        case CX_TOKEN(geoParentEntitiesQueryResult):
            switch (nElement)
            {
                case CX_TOKEN(geoParentEntitiesQuery):
                    mpCurParentResult->mxQuery.emplace();
                    mpCurParentResult->mxQuery->mosEntityId = rAttribs.getString(XML_entityId);
                    return nullptr;
                case CX_TOKEN(geoEntity):
                    mpCurParentResult->mxEntity.emplace();
                    mpCurParentResult->mxEntity->mosEntityName = rAttribs.getString(XML_entityName);
                    mpCurParentResult->mxEntity->mosEntityType = rAttribs.getString(XML_entityType);
                    return nullptr;
                case CX_TOKEN(geoParentEntity):
                    mpCurParentResult->mxParentEntity.emplace();
                    mpCurParentResult->mxParentEntity->mosEntityId
                        = rAttribs.getString(XML_entityId);
                    return nullptr;
            }
            break;
    }
    return nullptr;
}

// Handle character data
void GeographyContext::onCharacters(const OUString& rChars)
{
    switch (getCurrentElement())
    {
        case CX_TOKEN(binary):
            if (mrModel.mxGeoCache.has_value())
                mrModel.mxGeoCache->mosBinary = rChars;
            break;
        case CX_TOKEN(copyright):
            if (mpCurDataResult && mpCurDataResult->mxData.has_value())
                mpCurDataResult->mxData->maCopyrights.push_back({ rChars });
            break;
        case CX_TOKEN(entityType):
            if (mpCurChildResult && mpCurChildResult->mxQuery.has_value())
                mpCurChildResult->mxQuery->maEntityTypes.push_back(rChars);
            break;
    }
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
