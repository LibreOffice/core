/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bootstrap_process.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:37:20 $
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
#include <stdlib.h>
#include <stdio.h>

#ifndef _SAL_MAIN_H_
#include "sal/main.h"
#endif

#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

using namespace ::rtl;

// ----------------------------------- Main -----------------------------------
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    (void)argc;
    Bootstrap aBootstrap;
    //custom .ini/rc file
    Bootstrap aBs_custom( OUString::createFromAscii(argv[3]) );
    OUString suValue;
    OUString suDefault( OUString::createFromAscii("mydefault") );
    int flag = atoi( argv[1] );

    switch( flag ) {
    case 1:
        // parameters may be passed by command line arguments
        aBootstrap.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("UNO_SERVICES")),
            suValue );
        if (suValue.equalsAscii("service.rdb") )
        {
            return 10;
        }
        else
            return 11;
    case 2:
        // parameters may be passed by ini file
        aBootstrap.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("EXECUTABLE_RC")),
            suValue );
        if (suValue.equalsAscii("true") )
        {
            return 20;
        }
        else
            return 21;
    case 3:
        // parameters may be passed by command line arguments
        aBootstrap.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("QADEV_BOOTSTRAP")),
            suValue );
        if (suValue.equalsAscii("sun&ms") )
        {
            return 30;
        }
        else
            return 31;
    case 4:
        // parameters may be passed by custom .ini/rc file
        aBs_custom.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("RTLVALUE")),
            suValue );
        if (suValue.equalsAscii("qadev17") )
        {
            return 40;
        }
        else
            return 41;
    case 5:
        // parameters may be passed by inheritance
        aBs_custom.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("EXECUTABLE_RC")),
            suValue );
        if (suValue.equalsAscii("true") )
        {
            return 50;
        }
        else
            return 51;
    default:
        // parameters may be passed by inheritance
        aBs_custom.getFrom(
            OUString(RTL_CONSTASCII_USTRINGPARAM("ABCDE")),
            suValue, suDefault );
        if (suValue.equalsAscii("mydefault") )
        {
            return 60;
        }
        else
            return 61;
    }
}
