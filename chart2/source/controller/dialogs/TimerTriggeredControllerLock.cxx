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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "TimerTriggeredControllerLock.hxx"

// header for define EDIT_UPDATEDATA_TIMEOUT
#include <vcl/edit.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
