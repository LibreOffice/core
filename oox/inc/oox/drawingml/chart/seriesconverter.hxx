/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seriesconverter.hxx,v $
 *
 * $Revision: 1.2 $
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

#ifndef OOX_DRAWINGML_CHART_SERIESCONVERTER_HXX
#define OOX_DRAWINGML_CHART_SERIESCONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { class XDataSeries; }
    namespace chart2 { namespace data { class XLabeledDataSequence; } }
} } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct ErrorBarModel;

class ErrorBarConverter : public ConverterBase< ErrorBarModel >
{
public:
    explicit            ErrorBarConverter( const ConverterRoot& rParent, ErrorBarModel& rModel );
    virtual             ~ErrorBarConverter();

    /** Converts a OOXML errorbar and inserts it into the passed data series. */
    void                convertModelToDocument(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& rxDataSeries );
};

// ============================================================================

struct TrendlineModel;

class TrendlineConverter : public ConverterBase< TrendlineModel >
{
public:
    explicit            TrendlineConverter( const ConverterRoot& rParent, TrendlineModel& rModel );
    virtual             ~TrendlineConverter();

    /** Converts a OOXML trendline and inserts it into the passed data series. */
    void                convertModelToDocument(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& rxDataSeries );
};

// ============================================================================

struct SeriesModel;
class TypeGroupConverter;

class SeriesConverter : public ConverterBase< SeriesModel >
{
public:
    explicit            SeriesConverter( const ConverterRoot& rParent, SeriesModel& rModel );
    virtual             ~SeriesConverter();

    /** Creates a labeled data sequence object from category data link. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
                        createCategorySequence( const ::rtl::OUString& rRole );
    /** Creates a labeled data sequence object from value data link. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
                        createValueSequence( const ::rtl::OUString& rRole );
    /** Creates a data series object with initialized source links. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >
                        createDataSeries( const TypeGroupConverter& rTypeGroup );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
                        createLabeledDataSequence(
                            SeriesModel::SourceType eSourceType,
                            const ::rtl::OUString& rRole,
                            bool bUseTextLabel = false );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

