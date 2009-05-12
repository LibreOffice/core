/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartRangeSelectionListener.cxx,v $
 * $Revision: 1.3 $
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
//             if (pSh && pSh->ISA(ScTabViewShell))
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
