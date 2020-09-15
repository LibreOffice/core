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

#include "rangelst.hxx"

namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::uno { template <typename > class Sequence; }

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
    static void AdjustRangesOfChartsOnDestinationPage( const ScDocument& rSrcDoc, ScDocument& rDestDoc, const SCTAB nSrcTab, const SCTAB nDestTab );
    static void UpdateChartsOnDestinationPage( ScDocument* pDestDoc, const SCTAB nDestTab );
    static css::uno::Reference< css::chart2::XChartDocument > GetChartFromSdrObject( const SdrObject* pObject );
    static void GetChartRanges( const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc,
            std::vector< OUString >& rRanges );
    static void SetChartRanges( const css::uno::Reference< css::chart2::XChartDocument >& xChartDoc,
            const css::uno::Sequence< OUString >& rRanges );

    static void AddRangesIfProtectedChart( ScRangeListVector& rRangesVector, const ScDocument& rDocument, SdrObject* pObject );
    static void FillProtectedChartRangesVector( ScRangeListVector& rRangesVector, const ScDocument& rDocument, const SdrPage* pPage );
    static void GetChartNames( ::std::vector< OUString >& rChartNames, const SdrPage* pPage );
    static void CreateProtectedChartListenersAndNotify( ScDocument& rDoc, const SdrPage* pPage, ScModelObj* pModelObj, SCTAB nTab,
        const ScRangeListVector& rRangesVector, const ::std::vector< OUString >& rExcludedChartNames, bool bSameDoc = true );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
