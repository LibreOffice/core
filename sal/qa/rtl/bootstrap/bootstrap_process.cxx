/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <stdlib.h>
#include <stdio.h>
#include "sal/main.h"
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

using namespace ::rtl;


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    (void)argc;
    Bootstrap aBootstrap;
    
    Bootstrap aBs_custom( OUString::createFromAscii(argv[3]) );
    OUString suValue;
    OUString suDefault( "mydefault" );
    int flag = atoi( argv[1] );

    switch( flag ) {
    case 1:
        
        aBootstrap.getFrom(
            OUString("UNO_SERVICES"),
            suValue );
        if ( suValue == "service.rdb" )
        {
            return 10;
        }
        else
            return 11;
    case 2:
        
        aBootstrap.getFrom(
            OUString("EXECUTABLE_RC"),
            suValue );
        if ( suValue == "true" )
        {
            return 20;
        }
        else
            return 21;
    case 3:
        
        aBootstrap.getFrom(
            OUString("QADEV_BOOTSTRAP"),
            suValue );
        if ( suValue == "sun&ms" )
        {
            return 30;
        }
        else
            return 31;
    case 4:
        
        aBs_custom.getFrom(
            OUString("RTLVALUE"),
            suValue );
        if ( suValue == "qadev17" )
        {
            return 40;
        }
        else
            return 41;
    case 5:
        
        aBs_custom.getFrom(
            OUString("EXECUTABLE_RC"),
            suValue );
        if ( suValue == "true" )
        {
            return 50;
        }
        else
            return 51;
    default:
        
        aBs_custom.getFrom(
            OUString("ABCDE"),
            suValue, suDefault );
        if ( suValue == "mydefault" )
        {
            return 60;
        }
        else
            return 61;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
