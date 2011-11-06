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


#ifndef CHART2_CHARTDROPTARGETHELPER_HXX
#define CHART2_CHARTDROPTARGETHELPER_HXX

// for DropTargetHelper
#include <svtools/transfer.hxx>

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XChartDocument;
}
}}}

namespace chart
{

class ChartDropTargetHelper : public DropTargetHelper
{
public:
    explicit ChartDropTargetHelper(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >& rxDropTarget,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xChartDocument );
    virtual ~ChartDropTargetHelper();

protected:

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

private:
    // not available
    ChartDropTargetHelper();

    bool satisfiesPrerequisites() const;


    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDocument;
};

} //  namespace chart

// CHART2_CHARTDROPTARGETHELPER_HXX
#endif
