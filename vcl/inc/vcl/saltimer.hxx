/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saltimer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:09:17 $
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

#ifndef _SV_SALTIMER_HXX
#define _SV_SALTIMER_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SALWTYPE_HXX
#include <vcl/salwtype.hxx>
#endif

// ------------
// - SalTimer -
// ------------

/*
 * note: there will be only a single instance of SalTimer
 * SalTimer originally had only static methods, but
 * this needed to be virtualized for the sal plugin migration
 */

class VCL_DLLPUBLIC SalTimer
{
    SALTIMERPROC        m_pProc;
public:
    SalTimer() : m_pProc( NULL ) {}
    virtual ~SalTimer();

    // AutoRepeat and Restart
    virtual void            Start( ULONG nMS ) = 0;
    virtual void            Stop() = 0;

    // Callbacks (indepen in \sv\source\app\timer.cxx)
    void            SetCallback( SALTIMERPROC pProc )
    {
        m_pProc = pProc;
    }

    void            CallCallback()
    {
        if( m_pProc )
            m_pProc();
    }
};

#endif // _SV_SALTIMER_HXX
