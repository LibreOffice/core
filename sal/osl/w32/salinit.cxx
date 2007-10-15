/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:49:28 $
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
#include "precompiled_sal.hxx"

#include "system.h"

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes for initialization and deinitialization of SAL library

void SAL_CALL sal_detail_initialize(int argc, char ** argv)
{
    WSADATA wsaData;
    int     error;
    WORD    wVersionRequested;

    wVersionRequested = MAKEWORD(1, 1);

    error = WSAStartup(wVersionRequested, &wsaData);
    if ( 0 == error )
    {
        WORD wMajorVersionRequired = 1;
        WORD wMinorVersionRequired = 1;

        if ((LOBYTE(wsaData.wVersion) <  wMajorVersionRequired) ||
            (LOBYTE(wsaData.wVersion) == wMajorVersionRequired) &&
            ((HIBYTE(wsaData.wVersion) < wMinorVersionRequired)))
            {
                // How to handle a very unlikely error ???
            }
    }
    else
    {
        // How to handle a very unlikely error ???
    }

    osl_setCommandArgs(argc, argv);
}

void SAL_CALL sal_detail_deinitialize()
{
    if ( SOCKET_ERROR == WSACleanup() )
    {
        // We should never reach this point or we did wrong elsewhere
    }
}



#ifdef __cplusplus
}   // extern "C"
#endif
