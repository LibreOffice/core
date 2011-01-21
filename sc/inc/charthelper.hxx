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

#ifndef SC_CHARTHELPER_HXX
#define SC_CHARTHELPER_HXX

#include <tools/solar.h>
#include "address.hxx"
#include "global.hxx"
#include "rangelst.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>

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
