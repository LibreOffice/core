/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TimerTriggeredControllerLock.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:31:20 $
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

#include "TimerTriggeredControllerLock.hxx"

// header for define EDIT_UPDATEDATA_TIMEOUT
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;

TimerTriggeredControllerLock::TimerTriggeredControllerLock( const uno::Reference< frame::XModel >& xModel )
    : m_xModel( xModel )
    , m_apControllerLockGuard()
    , m_aTimer()
{
    m_aTimer.SetTimeout( 4*EDIT_UPDATEDATA_TIMEOUT );
    m_aTimer.SetTimeoutHdl( LINK( this, TimerTriggeredControllerLock, TimerTimeout ) );
}
TimerTriggeredControllerLock::~TimerTriggeredControllerLock()
{
    m_aTimer.Stop();
}

void TimerTriggeredControllerLock::startTimer()
{
    if(!m_apControllerLockGuard.get())
        m_apControllerLockGuard = std::auto_ptr< ControllerLockGuard >( new  ControllerLockGuard(m_xModel) );
    m_aTimer.Start();
}
IMPL_LINK( TimerTriggeredControllerLock, TimerTimeout, void*, EMPTYARG )
{
    m_apControllerLockGuard.reset();
    return 0;
}

//.............................................................................
} //namespace chart
//.............................................................................
