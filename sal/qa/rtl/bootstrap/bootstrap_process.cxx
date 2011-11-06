/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
