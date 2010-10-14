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
#include "precompiled_sal.hxx"
#include <stdlib.h>
#include <stdio.h>
#include "sal/main.h"
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
