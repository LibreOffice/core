/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

namespace oox
{
class PropertySet;
}

namespace oox::drawingml::chart
{
struct GeographyModel;
struct GeoClearModel;

void convertGeography(const GeographyModel& rModel, PropertySet& rSeriesProp);

css::uno::Sequence<css::beans::PropertyValue> geoClearToPropertyValues(const GeoClearModel& rModel);

GeoClearModel
geoClearFromPropertyValues(const css::uno::Sequence<css::beans::PropertyValue>& rProps);

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
