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

#ifndef OOX_DRAWINGML_CHART_DATASOURCECONVERTER_HXX
#define OOX_DRAWINGML_CHART_DATASOURCECONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { namespace data { class XDataSequence; } }
} } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct DataSequenceModel;

class DataSequenceConverter : public ConverterBase< DataSequenceModel >
{
public:
    explicit            DataSequenceConverter( const ConverterRoot& rParent, DataSequenceModel& rModel );
    virtual             ~DataSequenceConverter();

    /** Creates a data sequence object from the contained formula link. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
                        createDataSequence( const ::rtl::OUString& rRole );
};

// ============================================================================

struct DataSourceModel;

class DataSourceConverter : public ConverterBase< DataSourceModel >
{
public:
    explicit            DataSourceConverter( const ConverterRoot& rParent, DataSourceModel& rModel );
    virtual             ~DataSourceConverter();

    /** Creates a data sequence object from the contained series data. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
                        createDataSequence( const ::rtl::OUString& rRole );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

