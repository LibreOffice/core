/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TimerTriggeredControllerLock.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:56:57 $
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
#ifndef _CHART2_TIMERTRIGGEREDCONTROLLERLOCK_HXX
#define _CHART2_TIMERTRIGGEREDCONTROLLERLOCK_HXX

#include "ControllerLockGuard.hxx"

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

class TimerTriggeredControllerLock
{
public:
    TimerTriggeredControllerLock( const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >& xModel );
    virtual ~TimerTriggeredControllerLock();

    void startTimer();

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel >   m_xModel;
    std::auto_ptr< ControllerLockGuard >        m_apControllerLockGuard;
    AutoTimer       m_aTimer;

    DECL_LINK( TimerTimeout, void* );
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
