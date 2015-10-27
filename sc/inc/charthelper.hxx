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

#ifndef INCLUDED_SC_INC_CHARTHELPER_HXX
#define INCLUDED_SC_INC_CHARTHELPER_HXX

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
    static void AdjustRangesOfChartsOnDestinationPage( ScDocument* pSrcDoc, ScDocument* pDestDoc, const SCTAB nSrcTab, const SCTAB nDestTab );
    static void UpdateChartsOnDestinationPage( ScDocument* pDestDoc, const SCTAB nDestTab );
    static css::uno::Reference< css::chart2::XChartDocument > GetChartFromSdrObject( SdrObject* pObject );
    static void GetChartRanges( const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc,
            css::uno::Sequence< OUString >& rRanges );
    static void SetChartRanges( const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc,
            const css::uno::Sequence< OUString >& rRanges );

    static void AddRangesIfProtectedChart( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrObject* pObject );
    static void FillProtectedChartRangesVector( ScRangeListVector& rRangesVector, ScDocument* pDocument, SdrPage* pPage );
    static void GetChartNames( ::std::vector< OUString >& rChartNames, SdrPage* pPage );
    static void CreateProtectedChartListenersAndNotify( ScDocument* pDoc, SdrPage* pPage, ScModelObj* pModelObj, SCTAB nTab,
        const ScRangeListVector& rRangesVector, const ::std::vector< OUString >& rExcludedChartNames, bool bSameDoc = true );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
