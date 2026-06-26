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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_DATASOURCEMODEL_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_DATASOURCEMODEL_HXX

#include <map>

#include <com/sun/star/uno/Any.hxx>
#include <oox/drawingml/chart/modelbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

enum class SvNumFormatType : sal_Int16;

namespace oox::drawingml::chart {


struct DataSequenceModel
{
    typedef ::std::map< sal_Int32, css::uno::Any > AnyMap;

    AnyMap              maData;             /// Map of values, indexed by point identifier.
    OUString            maFormula;          /// Formula reference, e.g. into a spreadsheet.
    OUString            maNFormula;         /// Formula reference for a <cx:nf> element
    OUString            maFormatCode;       /// Number format for double values.
    sal_Int32           mnPointCount;       /// Number of points in this series source.
    sal_Int32           mnLevelCount;       /// Number of category levels.
    SvNumFormatType     meFormatType;       /// Type of number format in maFormatCode.

    explicit            DataSequenceModel();
                        ~DataSequenceModel();
};


struct DataSourceModel
{
    typedef ModelRef< DataSequenceModel > DataSequenceRef;

    DataSequenceRef     mxDataSeq;          /// The data sequence or formula link of this source.

    explicit            DataSourceModel();
                        ~DataSourceModel();
};

/// Dimension/data source type. The STR_ and NUM_ values correspond to chartex
/// ST_StringDimensionType and ST_NumericDimensionType, respectively. STR_CAT
/// and NUM_VAL are also used by traditional (non-chartex) charts.
enum class DataSourceType: sal_Int32
{
    UNKNOWN = 0,

    // String dimension types (strDim)
    STR_CAT,            /// "cat" - categories (also used by traditional charts)
    STR_COLORSTR,       /// "colorStr" - color strings
    STR_ENTITYID,       /// "entityId" - entity identifiers
    // Numeric dimension types (numDim)
    NUM_VAL,            /// "val" - numeric values (also used by traditional charts)
    NUM_X,              /// "x" - x-axis values
    NUM_Y,              /// "y" - y-axis values
    NUM_SIZE,           /// "size" - size values (e.g. sunburst, treemap)
    NUM_COLORVAL,       /// "colorVal" - color values (e.g. region map)
    // Non-dimension types (traditional charts only)
    POINTS,             /// Data point size (e.g. bubble size in bubble charts).
    DATALABELS,         /// Data point labels.
};


// Data source for chartex
struct DataSourceCxModel
{
    // Same as definition in SeriesModel
    typedef ModelMap< DataSourceType, DataSourceModel > DataSourceMap;
    typedef ModelMap<sal_Int32, DataSourceMap> DataMap;

    // Chartex data can have three levels of lists/maps:
    // 1. Multiple data series, each with a <cx:data> tag and indexed by id.
    // 2. Within a series, multiple sources ("val", "cat", etc.)
    // 3. Within a source, multiple points, with corresponding index.
    DataMap             maSourceMap; /// Data for chartex.

    explicit            DataSourceCxModel() = default;
};


/// Convert chartex XML attributes to a DataSourceType.
/// @param bNumeric  true for numDim, false for strDim
/// @param rType     the type attribute value (e.g. "val", "cat", "size")
DataSourceType dataSourceTypeFromCx(bool bNumeric, std::u16string_view rType);

/// Convert a DataSourceType back to chartex XML element/attribute info.
/// @param eType      the DataSourceType
/// @param rbNumeric  set to true for numDim, false for strDim
/// @param rsType     set to the type attribute value
/// @return false if eType is not a dimension type (POINTS, DATALABELS)
bool dataSourceTypeToCx(DataSourceType eType, bool& rbNumeric, OUString& rsType);

} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
