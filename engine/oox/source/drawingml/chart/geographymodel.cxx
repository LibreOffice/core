/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawingml/chart/geographymodel.hxx>

namespace oox::drawingml::chart
{
GeoProjectionType geoProjectionTypeFromString(std::u16string_view rStr)
{
    if (rStr == u"miller")
        return GeoProjectionType::Miller;
    if (rStr == u"robinson")
        return GeoProjectionType::Robinson;
    if (rStr == u"albers")
        return GeoProjectionType::Albers;
    return GeoProjectionType::Mercator;
}

OUString geoProjectionTypeToString(GeoProjectionType eType)
{
    switch (eType)
    {
        case GeoProjectionType::Miller:
            return u"miller"_ustr;
        case GeoProjectionType::Robinson:
            return u"robinson"_ustr;
        case GeoProjectionType::Albers:
            return u"albers"_ustr;
        case GeoProjectionType::Mercator:
            return u"mercator"_ustr;
    }
    return u"mercator"_ustr;
}

GeoMappingLevel geoMappingLevelFromString(std::u16string_view rStr)
{
    if (rStr == u"postalCode")
        return GeoMappingLevel::PostalCode;
    if (rStr == u"county")
        return GeoMappingLevel::County;
    if (rStr == u"state")
        return GeoMappingLevel::State;
    if (rStr == u"countryRegion")
        return GeoMappingLevel::CountryRegion;
    if (rStr == u"countryRegionList")
        return GeoMappingLevel::CountryRegionList;
    if (rStr == u"world")
        return GeoMappingLevel::World;
    if (rStr == u"autodetect")
        return GeoMappingLevel::Autodetect;
    return GeoMappingLevel::DataOnly;
}

OUString geoMappingLevelToString(GeoMappingLevel eLevel)
{
    switch (eLevel)
    {
        case GeoMappingLevel::PostalCode:
            return u"postalCode"_ustr;
        case GeoMappingLevel::County:
            return u"county"_ustr;
        case GeoMappingLevel::State:
            return u"state"_ustr;
        case GeoMappingLevel::CountryRegion:
            return u"countryRegion"_ustr;
        case GeoMappingLevel::CountryRegionList:
            return u"countryRegionList"_ustr;
        case GeoMappingLevel::World:
            return u"world"_ustr;
        case GeoMappingLevel::Autodetect:
            return u"autodetect"_ustr;
        case GeoMappingLevel::DataOnly:
            return u"dataOnly"_ustr;
    }
    return u"dataOnly"_ustr;
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
