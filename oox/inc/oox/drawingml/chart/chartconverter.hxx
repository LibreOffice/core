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

#ifndef OOX_DRAWINGML_CHART_CHARTCONVERTER_HXX
#define OOX_DRAWINGML_CHART_CHARTCONVERTER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <oox/dllapi.h>

namespace com { namespace sun { namespace star {
    namespace chart2 { class XChartDocument; }
    namespace chart2 { namespace data { class XDataProvider; } }
    namespace chart2 { namespace data { class XDataSequence; } }
} } }

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox {
namespace drawingml {
namespace chart {

struct ChartSpaceModel;
struct DataSequenceModel;

// ============================================================================

class OOX_DLLPUBLIC ChartConverter
{
public:
    explicit            ChartConverter();
    virtual             ~ChartConverter();

    /** Converts the passed OOXML chart model to the passed chart2 document. */
    void                convertFromModel(
                            ::oox::core::XmlFilterBase& rFilter,
                            ChartSpaceModel& rModel,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc );

    /** Creates an internal data provider. Derived classes may override this
        function to create an external data provider. */
    virtual void        createDataProvider(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc );

    /** Creates a data sequence from a formula. Dummy implementation. Derived
        classes have to override this function to actually parse the formula. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
                        createDataSequence(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >& rxDataProvider,
                            const DataSequenceModel& rDataSeq );

private:
                        ChartConverter( const ChartConverter& );
    ChartConverter&     operator=( const ChartConverter& );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

