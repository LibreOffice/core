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
