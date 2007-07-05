/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: saltimer.h,v $
*
*  $Revision: 1.3 $
*
*  last change: $Author: rt $ $Date: 2007-07-05 08:17:20 $
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

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#ifndef _SV_SALTIMER_HXX
#include <saltimer.hxx>
#endif

class AquaSalTimer : public SalTimer
{
  public:

    AquaSalTimer();
    virtual ~AquaSalTimer();

    void Start( ULONG nMS );
    void Stop();

};

#endif
