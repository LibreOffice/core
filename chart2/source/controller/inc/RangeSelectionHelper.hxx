/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef CHART2_RANGESELECTIONHELPER_HXX
#define CHART2_RANGESELECTIONHELPER_HXX

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
        const ::rtl::OUString & aCurrentRange,
        const ::rtl::OUString & aUIString,
        RangeSelectionListenerParent & rListenerParent );
    void stopRangeListening( bool bRemoveListener = true );
    bool verifyCellRange( const ::rtl::OUString & rRangeStr );
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

} //  namespace chart

// CHART2_RANGESELECTIONHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
