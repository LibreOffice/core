/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: saltimer.h,v $
*
*  $Revision: 1.7 $
*
*  last change: $Author: kz $ $Date: 2008-04-04 10:57:17 $
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

#ifndef _SV_SALTIMER_H
#define _SV_SALTIMER_H

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#include "vcl/saltimer.hxx"

class AquaSalTimer : public SalTimer
{
  public:

    AquaSalTimer();
    virtual ~AquaSalTimer();

    void Start( ULONG nMS );
    void Stop();

    static void handleStartTimerEvent( NSEvent* pEvent );


    static NSTimer* pRunningTimer;
    static bool bDispatchTimer;
};

#endif
