/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_DRAWINGML_CHART_DATASOURCEMODEL_HXX
#define OOX_DRAWINGML_CHART_DATASOURCEMODEL_HXX

#include <com/sun/star/uno/Any.hxx>
#include "oox/drawingml/chart/modelbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct DataSequenceModel
{
    typedef ::std::map< sal_Int32, ::com::sun::star::uno::Any > AnyMap;

    AnyMap              maData;             /// Map of values, indexed by point identifier.
    ::rtl::OUString     maFormula;          /// Formula reference, e.g. into a spreadsheet.
    ::rtl::OUString     maFormatCode;       /// Number format for double values.
    sal_Int32           mnPointCount;       /// Number of points in this series source.

    explicit            DataSequenceModel();
                        ~DataSequenceModel();
};

// ============================================================================

struct DataSourceModel
{
    typedef ModelRef< DataSequenceModel > DataSequenceRef;

    DataSequenceRef     mxDataSeq;          /// The data sequence or formula link of this source.

    explicit            DataSourceModel();
                        ~DataSourceModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
