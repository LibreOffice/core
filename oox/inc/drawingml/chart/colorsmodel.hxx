/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <variant>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/chart/modelbase.hxx>
#include <oox/drawingml/shape.hxx>
#include <docmodel/styles/ChartColorStyle.hxx>

namespace oox::drawingml::chart
{
// Holds the contents of a cs:CT_ColorStyle
struct ColorStyleModel
{
    std::vector<Color> maColors;
    std::vector<model::ComplexColor> maComplexColors;
    ::model::ColorStyleMethod meMethod;
    sal_Int32 mnId;
};

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
