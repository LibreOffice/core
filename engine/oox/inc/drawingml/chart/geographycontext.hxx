/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <oox/core/contexthandler2.hxx>

namespace oox::drawingml::chart
{
struct GeographyModel;
struct GeoLocationQueryResultModel;
struct GeoDataEntityQueryResultModel;
struct GeoDataPointToEntityQueryResultModel;
struct GeoChildEntitiesQueryResultModel;
struct GeoParentEntitiesQueryResultModel;

class GeographyContext final : public ::oox::core::ContextHandler2
{
public:
    explicit GeographyContext(::oox::core::ContextHandler2Helper& rParent, GeographyModel& rModel);

    virtual ::oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                           const AttributeList& rAttribs) override;
    virtual void onCharacters(const OUString& rChars) override;

private:
    GeographyModel& mrModel;
    GeoLocationQueryResultModel* mpCurLocResult = nullptr;
    GeoDataEntityQueryResultModel* mpCurDataResult = nullptr;
    GeoDataPointToEntityQueryResultModel* mpCurPtResult = nullptr;
    GeoChildEntitiesQueryResultModel* mpCurChildResult = nullptr;
    GeoParentEntitiesQueryResultModel* mpCurParentResult = nullptr;
};

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
