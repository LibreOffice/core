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

#ifndef _SV_SALTIMER_HXX
#define _SV_SALTIMER_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/salwtype.hxx>

// ------------
// - SalTimer -
// ------------

/*
 * note: there will be only a single instance of SalTimer
 * SalTimer originally had only static methods, but
 * this needed to be virtualized for the sal plugin migration
 */

class VCL_PLUGIN_PUBLIC SalTimer
{
    SALTIMERPROC        m_pProc;
public:
    SalTimer() : m_pProc( NULL ) {}
    virtual ~SalTimer();

    // AutoRepeat and Restart
    virtual void            Start( sal_uLong nMS ) = 0;
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
