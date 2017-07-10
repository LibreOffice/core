/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdio.h>
#include "sal/main.h"
#include <rtl/process.h>
#include <rtl_Process_Const.h>

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

    if ( suArg[0] != suParam0 ||
         suArg[1] != suParam1 ||
         suArg[2] != suParam2 ||
         suArg[3] != suParam3 )
    {
        return 0;
    }
    return 2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
