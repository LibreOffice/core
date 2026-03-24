/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <optional>
#include <vector>
#include <rtl/ustring.hxx>

namespace oox::drawingml::chart
{
struct GeoAddressModel
{
    std::optional<OUString> mosAddress1;
    std::optional<OUString> mosCountryRegion;
    std::optional<OUString> mosAdminDistrict1;
    std::optional<OUString> mosAdminDistrict2;
    std::optional<OUString> mosPostalCode;
    std::optional<OUString> mosLocality;
    std::optional<OUString> mosIsoCountryCode;
};

struct GeoLocationQueryModel
{
    std::optional<OUString> mosCountryRegion;
    std::optional<OUString> mosAdminDistrict1;
    std::optional<OUString> mosAdminDistrict2;
    std::optional<OUString> mosPostalCode;
    std::optional<OUString> mosEntityType;
};

struct GeoLocationModel
{
    std::optional<OUString> mosLatitude;
    std::optional<OUString> mosLongitude;
    std::optional<OUString> mosEntityName;
    std::optional<OUString> mosEntityType;
    std::optional<GeoAddressModel> mxAddress;
};

struct GeoLocationQueryResultModel
{
    std::optional<GeoLocationQueryModel> mxQuery;
    std::vector<GeoLocationModel> maLocations;
};

struct GeoDataEntityQueryModel
{
    std::optional<OUString> mosEntityType;
    std::optional<OUString> mosEntityId;
};

struct GeoPolygonModel
{
    std::optional<OUString> mosPolygonId;
    std::optional<OUString> mosNumPoints;
    std::optional<OUString> mosPcaRings;
};

struct GeoCopyrightModel
{
    OUString msText;
};

struct GeoDataModel
{
    std::optional<OUString> mosEntityName;
    std::optional<OUString> mosEntityId;
    std::optional<OUString> mosEast;
    std::optional<OUString> mosWest;
    std::optional<OUString> mosNorth;
    std::optional<OUString> mosSouth;
    std::vector<GeoPolygonModel> maPolygons;
    std::vector<GeoCopyrightModel> maCopyrights;
};

struct GeoDataEntityQueryResultModel
{
    std::optional<GeoDataEntityQueryModel> mxQuery;
    std::optional<GeoDataModel> mxData;
};

struct GeoDataPointQueryModel
{
    std::optional<OUString> mosEntityType;
    std::optional<OUString> mosLatitude;
    std::optional<OUString> mosLongitude;
};

struct GeoDataPointToEntityQueryModel
{
    std::optional<OUString> mosEntityType;
    std::optional<OUString> mosEntityId;
};

struct GeoDataPointToEntityQueryResultModel
{
    std::optional<GeoDataPointQueryModel> mxPointQuery;
    std::optional<GeoDataPointToEntityQueryModel> mxEntityQuery;
};

struct GeoChildEntitiesQueryModel
{
    std::optional<OUString> mosEntityId;
    std::vector<OUString> maEntityTypes;
};

struct GeoHierarchyEntityModel
{
    std::optional<OUString> mosEntityName;
    std::optional<OUString> mosEntityId;
    std::optional<OUString> mosEntityType;
};

struct GeoChildEntitiesQueryResultModel
{
    std::optional<GeoChildEntitiesQueryModel> mxQuery;
    std::vector<GeoHierarchyEntityModel> maEntities;
};

struct GeoParentEntitiesQueryModel
{
    std::optional<OUString> mosEntityId;
};

struct GeoEntityModel
{
    std::optional<OUString> mosEntityName;
    std::optional<OUString> mosEntityType;
};

struct GeoParentEntityModel
{
    std::optional<OUString> mosEntityId;
};

struct GeoParentEntitiesQueryResultModel
{
    std::optional<GeoParentEntitiesQueryModel> mxQuery;
    std::optional<GeoEntityModel> mxEntity;
    std::optional<GeoParentEntityModel> mxParentEntity;
};

struct GeoClearModel
{
    std::vector<GeoLocationQueryResultModel> maLocResults;
    std::vector<GeoDataEntityQueryResultModel> maDataResults;
    std::vector<GeoDataPointToEntityQueryResultModel> maPtResults;
    std::vector<GeoChildEntitiesQueryResultModel> maChildResults;
    std::vector<GeoParentEntitiesQueryResultModel> maParentResults;
};

struct GeoCacheModel
{
    std::optional<OUString> mosProvider;
    std::optional<OUString> mosBinary;
    std::optional<GeoClearModel> mxClear;
};

struct GeographyModel
{
    std::optional<OUString> mosProjectionType;
    std::optional<OUString> mosViewedRegionType;
    std::optional<OUString> mosCultureLanguage;
    std::optional<OUString> mosCultureRegion;
    std::optional<OUString> mosAttribution;
    std::optional<GeoCacheModel> mxGeoCache;
};

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
