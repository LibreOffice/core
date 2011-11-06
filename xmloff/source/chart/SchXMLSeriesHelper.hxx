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


#ifndef _XMLOFF_SCH_XML_SERIESHELPER_HXX
#define _XMLOFF_SCH_XML_SERIESHELPER_HXX

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vector>
#include <map>

class SchXMLSeriesHelper
{
public:
    static ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >
            getDataSeriesFromDiagram(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram > & xDiagram );
    static ::std::map< ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >, sal_Int32 >
            getDataSeriesIndexMapFromDiagram(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram > & xDiagram );

    static bool isCandleStickSeries(
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDataSeries >& xSeries
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xChartModel  );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > getFirstCandleStickSeries(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDiagram > & xDiagram  );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > createOldAPISeriesPropertySet(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries >& xSeries
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xChartModel );

    static ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > createOldAPIDataPointPropertySet(
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries >& xSeries
                    , sal_Int32 nPointIndex
                    , const ::com::sun::star::uno::Reference<
                        ::com::sun::star::frame::XModel >& xChartModel );
};

// _XMLOFF_SCH_XML_SERIESHELPER_HXX
#endif
