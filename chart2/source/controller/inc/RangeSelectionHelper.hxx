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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RANGESELECTIONHELPER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RANGESELECTIONHELPER_HXX

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace com { namespace sun { namespace star {
    namespace sheet{
        class XRangeSelection;
        class XRangeSelectionListener;
        class XCellRangesAccess;
    }
}}}

namespace chart
{

class RangeSelectionListenerParent;

class RangeSelectionHelper
{
public:
    explicit RangeSelectionHelper(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument );
    ~RangeSelectionHelper();

    bool hasRangeSelection();
    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelection > getRangeSelection();
    void raiseRangeSelectionDocument();
    bool chooseRange(
        const OUString & aCurrentRange,
        const OUString & aUIString,
        RangeSelectionListenerParent & rListenerParent );
    void stopRangeListening( bool bRemoveListener = true );
    bool verifyCellRange( const OUString & rRangeStr );
    bool verifyArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArguments );

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelection >
        m_xRangeSelection;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >
        m_xChartDocument;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XCellRangesAccess >
        m_xCellRangesAccess;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelectionListener >
        m_xRangeSelectionListener;
};

}

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_RANGESELECTIONHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
