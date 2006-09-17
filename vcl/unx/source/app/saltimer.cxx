/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saltimer.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:35:56 $
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
#include "precompiled_vcl.hxx"

#include <stdio.h>
#if defined(IRIX)
#include <ctime>
#endif
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#include <salunx.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALTIMER_H
#include <saltimer.h>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif

// -=-= SalData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalData::Timeout() const
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpSalTimer )
        pSVData->mpSalTimer->CallCallback();
}

// -=-= SalXLib =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalXLib::StopTimer()
{
    m_aTimeout.tv_sec   = 0;
    m_aTimeout.tv_usec  = 0;
    m_nTimeoutMS        = 0;
}

void SalXLib::StartTimer( ULONG nMS )
{
    timeval Timeout (m_aTimeout); // previous timeout.
    gettimeofday (&m_aTimeout, 0);

    m_nTimeoutMS  = nMS;
    m_aTimeout    += m_nTimeoutMS;

    if ((Timeout > m_aTimeout) || (Timeout.tv_sec == 0))
    {
        // Wakeup from previous timeout (or stopped timer).
        Wakeup();
    }
}

// -=-= SalTimer -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalTimer* X11SalInstance::CreateSalTimer()
{
    return new X11SalTimer();
}

X11SalTimer::~X11SalTimer()
{
}

void X11SalTimer::Stop()
{
    GetX11SalData()->GetLib()->StopTimer();
}

void X11SalTimer::Start( ULONG nMS )
{
    GetX11SalData()->GetLib()->StartTimer( nMS );
}

