/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

