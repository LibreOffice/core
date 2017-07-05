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

#include <rtl/process.h>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/byteseq.hxx>

#include <osl/process.h>

int main( int argc, char *argv[] )
{
    osl_setCommandArgs (argc, argv);

    sal_Int32 nCount = rtl_getAppCommandArgCount();

#if OSL_DEBUG_LEVEL > 0
    OUStringBuffer debugBuff;
    debugBuff.append("rtl-commandargs (").append(nCount).append(")real args: ").append(argc);
    for( sal_Int32 i = 0 ; i < nCount ; i ++ )
    {
        OUString data;
        rtl_getAppCommandArg( i , &(data.pData) );
        OString o = OUStringToOString( data, RTL_TEXTENCODING_ASCII_US );
        debugBuff.append(" ").append(o);
    }
    SAL_INFO("sal.test", debugBuff.toString());
#endif

    if( nCount == 0 )
    {
        printf( "usage : testbootstrap <checkedValueOfMyBootstrapValue>\n" );
          exit( 1 );
    }

    OUString iniName;
    Bootstrap::get(OUString("iniName"), iniName, OUString());

    SAL_INFO_IF(!iniName.isEmpty(), "sal.test", "using ini: " << iniName);

    Bootstrap bootstrap(iniName);

    OUString name( "MYBOOTSTRAPTESTVALUE" );
    OUString myDefault("$Default");

    OUString value;
    sal_Bool useDefault;

    OUString aDummy;
    useDefault = bootstrap.getFrom(OUString("USEDEFAULT"), aDummy);

    sal_Bool result = sal_False;
    sal_Bool found  = sal_True;

    if(useDefault)
        bootstrap.getFrom(name, value, myDefault);

    else
        found = bootstrap.getFrom(name, value);

    if(found)
    {
        OUString para(OUString::createFromAscii( argv[1] ));

        result = para == value;

        if(!result)
        {
            OString para_tmp = OUStringToOString(para, RTL_TEXTENCODING_ASCII_US);
            OString value_tmp = OUStringToOString(value, RTL_TEXTENCODING_ASCII_US);

            fprintf(stderr, "para(%s) != value(%s)\n", para_tmp.getStr(), value_tmp.getStr());
        }
    }
    else
        fprintf(stderr, "bootstrap parameter couldn't be found\n");

    // test the default case
    name = "no_one_has_set_this_name";
      assert( ! bootstrap.getFrom( name, value ) );
    result = result && !bootstrap.getFrom( name, value );

    myDefault = "1";
    OUString myDefault2 = "2";

    bootstrap.getFrom( name, value, myDefault );
      assert( value == myDefault );
    result = result && (value == myDefault);

    bootstrap.getFrom( name, value, myDefault2 );
      assert( value == myDefault2 );
    result = result && (value == myDefault2);

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
