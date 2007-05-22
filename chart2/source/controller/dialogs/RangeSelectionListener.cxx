/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RangeSelectionListener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:29:27 $
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
#include "precompiled_chart2.hxx"

#include "RangeSelectionListener.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

RangeSelectionListener::RangeSelectionListener(
    RangeSelectionListenerParent & rParent,
    const ::rtl::OUString & rInitialRange,
    const Reference< frame::XModel >& xModelToLockController ) :
        m_rParent( rParent ),
        m_aRange( rInitialRange ),
        m_aControllerLockGuard( xModelToLockController )
{}

RangeSelectionListener::~RangeSelectionListener()
{}

// ____ XRangeSelectionListener ____
void SAL_CALL RangeSelectionListener::done( const sheet::RangeSelectionEvent& aEvent )
    throw (uno::RuntimeException)
{
    m_aRange = aEvent.RangeDescriptor;
    m_rParent.listeningFinished( m_aRange );
}

void SAL_CALL RangeSelectionListener::aborted( const sheet::RangeSelectionEvent& aEvent )
    throw (uno::RuntimeException)
{
    m_rParent.listeningFinished( m_aRange );
}

// ____ XEventListener ____
void SAL_CALL RangeSelectionListener::disposing( const lang::EventObject& Source )
    throw (uno::RuntimeException)
{
    m_rParent.disposingRangeSelection();
}

} //  namespace chart
