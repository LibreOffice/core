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



#ifndef SC_CHARTHELPER_HXX
#define SC_CHARTHELPER_HXX

#include <tools/solar.h>
#include "address.hxx"
#include "global.hxx"
#include "rangelst.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>

#include <vector>

class SdrObject;
class SdrPage;
class ScModelObj;

typedef ::std::vector< ScRangeList > ScRangeListVector;

/** Use this to handle charts in a calc document
*/
class ScChartHelper
{
public:
    static sal_uInt16 DoUpdateAllCharts( ScDocument* pDoc );
    static sal_uInt16 DoUpdateCharts( const ScAddress& rPos, ScDocument* pDoc ); //use this to replace ScDBFunc::DoUpdateCharts in future
    static void AdjustRangesOfChartsOnDestinationPage( ScDocument* pSrcDoc, ScDocument* pDestDoc, const SCTAB nSrcTab, const SCTAB nDestTab );
    static void UpdateChartsOnDestinationPage( ScDocument* pDestDoc, const SCTAB nDestTab );
    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > GetChartFromSdrObject( SdrObject* pObject );
    static void GetChartRanges( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDoc,
            ::com::sun::star::uno::Sequence< rtl::OUString >& rRanges );
    static void SetChartRanges( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDoc,
            const ::com::sun::star::uno::Sequence< rtl::OUString >& rRanges );

    static void AddRangesIfProtectedChart( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrObject* pObject );
    static void FillProtectedChartRangesVector( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrPage* pPage );
    static void GetChartNames( ::std::vector< ::rtl::OUString >& rChartNames, SdrPage* pPage );
    static void CreateProtectedChartListenersAndNotify( ScDocument* pDoc, SdrPage* pPage, ScModelObj* pModelObj, SCTAB nTab,
        const ScRangeListVector& rRangesVector, const ::std::vector< ::rtl::OUString >& rExcludedChartNames, bool bSameDoc = true );
};

#endif
