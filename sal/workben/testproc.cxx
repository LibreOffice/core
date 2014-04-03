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

#include <osl/process.h>
#include <stdio.h>

#if defined WNT
    #define MAIN _cdecl main
#else
    #define MAIN main
#endif

int MAIN (void)
{
    //oslProcess Process;
    sal_uInt32 nArgCount;
    sal_uInt32 index;
    rtl_uString* ustrExeFile=0;
    rtl_uString* ustrArg=0;

    nArgCount=osl_getCommandArgCount();

    fprintf(stderr,"Arg Count == %i\n\n",nArgCount);

    for ( index = 0 ; index <= nArgCount+1 ; index++ )
    {
        fprintf(stderr,"Getting Arg No . %i\n",index);

        osl_getCommandArg(index,&ustrArg);

        fprintf(stderr,"done ...\n\n");
    }

    osl_getExecutableFile(&ustrExeFile);

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
