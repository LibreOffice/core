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
#include <stdlib.h>
#include <string.h>
#include "sal/main.h"
#include "sal/types.h"
#include "osl/thread.h"
#include "rtl/ustring.hxx"
#include "rtl/byteseq.hxx"
#include "jvmfwk/framework.h"

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

static sal_Bool hasOption(char const * szOption, int argc, char** argv);
static rtl::OString getLD_LIBRARY_PATH(const rtl::ByteSequence & vendorData);
static bool findAndSelect(JavaInfo**);

#define HELP_TEXT    \
"\njavaldx is necessary to make Java work on some UNIX platforms." \
"It prints a string to std out that consists of directories which " \
"have to be included into the LD_LIBRARY_PATH variable.The setting of " \
"the variable usually occurs in a shell script that runs javaldx.\n" \
"The directories are from the chosen java installation. \n" \
"Options are: \n"\
"--help or -h\n"

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    if( hasOption("--help",argc, argv) || hasOption("-h", argc, argv))
    {
        fprintf(stdout, HELP_TEXT);// default
        return 0;
    }
    javaFrameworkError errcode = JFW_E_NONE;
    sal_Bool bEnabled = sal_False;
    errcode = jfw_getEnabled( & bEnabled);
    if (errcode == JFW_E_NONE && bEnabled == sal_False)
    {
            //Do not do any preparation because that may only slow startup time.
        return 0;
    }
    else if (errcode != JFW_E_NONE && errcode != JFW_E_DIRECT_MODE)
    {
        fprintf(stderr,"javaldx failed! \n");
        return -1;
    }


    JavaInfo * pInfo = NULL;
    errcode = jfw_getSelectedJRE( & pInfo);

    if (errcode != JFW_E_NONE && errcode != JFW_E_INVALID_SETTINGS)
    {
        fprintf(stderr,"javaldx failed! \n");
        return -1;
    }

    if (pInfo == NULL)
    {
        if (false == findAndSelect(&pInfo))
            return -1;
    }
    else
    {
        //check if the JRE was not uninstalled
        sal_Bool bExist = sal_False;
        errcode = jfw_existJRE(pInfo, &bExist);
        if (errcode == JFW_E_NONE)
        {
            if (!bExist && !findAndSelect(&pInfo))
                return -1;
        }
        else
        {
            fprintf(stderr, "javaldx: Could not determine if JRE still exist\n");
            return -1;
        }
    }

    rtl::OUString aVendor( pInfo->sVendor );
    // Only do something if the sunjavaplugin created this JavaInfo
    if ( aVendor != "Sun Microsystems Inc." &&
         aVendor != "Oracle Corporation" &&
         aVendor != "IBM Corporation" &&
         aVendor != "Blackdown Java-Linux Team" &&
         aVendor != "Apple Inc." &&
         aVendor != "Apple Computer, Inc." &&
         aVendor != "BEA Systems, Inc." &&
         aVendor != "Free Software Foundation, Inc." &&
         aVendor != "The FreeBSD Foundation" )
        return 0;

    rtl::OString sPaths = getLD_LIBRARY_PATH(pInfo->arVendorData);
    fprintf(stdout, "%s\n", sPaths.getStr());
    jfw_freeJavaInfo(pInfo);

    return 0;
}

rtl::OString getLD_LIBRARY_PATH(const rtl::ByteSequence & vendorData)
{
    const sal_Unicode* chars = (sal_Unicode*) vendorData.getConstArray();
    sal_Int32 len = vendorData.getLength();
    rtl::OUString sData(chars, len / 2);
    //the runtime lib is on the first line
    sal_Int32 index = 0;
    rtl::OUString aToken = sData.getToken( 1, '\n', index);

    rtl::OString paths =
        rtl::OUStringToOString(aToken, osl_getThreadTextEncoding());
    return paths;
}

static sal_Bool hasOption(char const * szOption, int argc, char** argv)
{
    sal_Bool retVal= sal_False;
    for(sal_Int16 i= 1; i < argc; i++)
    {
        if( ! strcmp(argv[i], szOption))
        {
            retVal= sal_True;
            break;
        }
    }
    return retVal;
}

static bool findAndSelect(JavaInfo ** ppInfo)
{
    javaFrameworkError errcode = jfw_findAndSelectJRE(ppInfo);
    if (errcode == JFW_E_NO_JAVA_FOUND)
    {
        fprintf(stderr,"javaldx: Could not find a Java Runtime Environment! \n");
        return false;
    }
    else if (errcode != JFW_E_NONE && errcode != JFW_E_DIRECT_MODE)
    {
        fprintf(stderr,"javaldx failed!\n");
        return false;
    }
    return true;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
