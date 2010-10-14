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

#include <stdio.h>
#include "sal/main.h"
#include <rtl/process.h>
#include <rtl_Process_Const.h>

using namespace ::rtl;

// ----------------------------------- Main -----------------------------------
SAL_IMPLEMENT_MAIN_WITH_ARGS(, argv)
{
    printf("# %s is called.\n", argv[0]);

    sal_Int32 nCount = rtl_getAppCommandArgCount();
    if ( nCount != 4 )
    {
        printf(
            "# not enough arguments found, need 4 found %ld.\n",
            sal::static_int_cast< long >(nCount));
        return 0;
    }

    OUString suArg[4];
    for( sal_Int32 i = 0 ; i < nCount ; i ++ )
    {
        rtl_getAppCommandArg( i , &(suArg[i].pData) );
        rtl::OString aString;
        aString = ::rtl::OUStringToOString( suArg[i], RTL_TEXTENCODING_ASCII_US );
        printf(
            "# Parameter[%ld] is %s\n", sal::static_int_cast< long >(i),
            aString.getStr());
    }

    if ( suArg[0].compareTo( suParam0) != 0 ||
         suArg[1].compareTo( suParam1) != 0 ||
         suArg[2].compareTo( suParam2) != 0 ||
         suArg[3].compareTo( suParam3) != 0 )
    {
        return 0;
    }
    return 2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
