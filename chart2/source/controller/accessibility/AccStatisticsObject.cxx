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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "AccStatisticsObject.hxx"
#include <vcl/svapp.hxx>
#include <svx/svditer.hxx>

using ::rtl::OUString;

namespace chart
{

AccStatisticsObject::AccStatisticsObject( const AccessibleElementInfo& rAccInfo )
        : AccessibleChartElement( rAccInfo, false/*NoChildren*/, true/*AlwaysTransparent*/ )
        //, m_eType( eType )
        //, m_nSeriesIndex( nSeriesIndex )
{
//    ChartModel * pModel = GetChartModel();
//    OSL_ASSERT( pModel );

    // /-- solar
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
/*    switch( eType )
    {
        case MEAN_VAL_LINE:
            SetItemSet( pModel->GetAverageAttr( m_nSeriesIndex ));
            break;
        case ERROR_BARS:
            SetItemSet( pModel->GetErrorAttr( m_nSeriesIndex ));
            break;
        case REGRESSION:
            SetItemSet( pModel->GetRegressAttr( m_nSeriesIndex ));
            break;
    }*/
    // \-- solar
}

AccStatisticsObject::~AccStatisticsObject()
{
}

OUString SAL_CALL AccStatisticsObject::getAccessibleName()
    throw (::com::sun::star::uno::RuntimeException)
{
    return getToolTipText();
}

OUString SAL_CALL AccStatisticsObject::getImplementationName()
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "StatisticsObject" ));
}

}  // namespace chart
