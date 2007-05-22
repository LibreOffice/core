/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartRangeSelectionListener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 20:10:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
