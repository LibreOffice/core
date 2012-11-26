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
#include "precompiled_sc.hxx"

#include "ChartRangeSelectionListener.hxx"

#include <com/sun/star/chart2/data/XRangeHighlighter.hpp>

#include <sfx2/viewfrm.hxx>
#include "tabvwsh.hxx"
#include "unonames.hxx"
#include "miscuno.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

SC_SIMPLE_SERVICE_INFO( ScChartRangeSelectionListener, "ScChartRangeSelectionListener",
                        SC_SERVICENAME_CHRANGEHILIGHT )

ScChartRangeSelectionListener::ScChartRangeSelectionListener( ScTabViewShell * pViewShell ) :
        ScChartRangeSelectionListener_Base( m_aMutex ),
        m_pViewShell( pViewShell )
{}

ScChartRangeSelectionListener::~ScChartRangeSelectionListener()
{}

// ____ XModifyListener ____
void SAL_CALL ScChartRangeSelectionListener::selectionChanged( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    Reference< chart2::data::XRangeHighlighter > xRangeHighlighter( aEvent.Source, uno::UNO_QUERY );
    if( xRangeHighlighter.is())
    {
        Sequence< chart2::data::HighlightedRange > aRanges( xRangeHighlighter->getSelectedRanges());

        // search the view on which the chart is active

        if( m_pViewShell )
        {
            m_pViewShell->DoChartSelection( aRanges );
        }
//         SfxViewFrame *pFrame = SfxViewFrame::GetFirst( m_pDocShell );
//         while (pFrame)
//         {
//             SfxViewShell* pSh = pFrame->GetViewShell();
//             if (pSh && dynamic_cast< ScTabViewShell* >(pSh))
//             {
//                 ScTabViewShell* pViewSh  = (ScTabViewShell*)pSh;
//             }
//             pFrame = SfxViewFrame::GetNext( *pFrame, m_pDocShell );
//         }
    }
}

// ____ XEventListener ____
void SAL_CALL ScChartRangeSelectionListener::disposing( const lang::EventObject& /*Source*/ )
    throw (uno::RuntimeException)
{
}

// ____ WeakComponentImplHelperBase ____
void SAL_CALL ScChartRangeSelectionListener::disposing()
{
    m_pViewShell = 0;
}
