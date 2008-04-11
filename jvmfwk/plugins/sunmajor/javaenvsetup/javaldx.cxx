/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: javaldx.cxx,v $
 * $Revision: 1.15 $
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
#include "precompiled_jvmfwk.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sal/main.h"
#include "sal/types.h"
#include "osl/thread.h"
#include "rtl/ustring.hxx"
#include "rtl/byteseq.hxx"
#include "jvmfwk/framework.h"

using namespace rtl;

#define OUSTR(x) OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

static sal_Bool hasOption(char const * szOption, int argc, char** argv);
static rtl::OString getLD_LIBRARY_PATH(const rtl::ByteSequence & vendorData);
//static sal_Bool printPaths(const OUString& sPathFile);

#ifdef MACOSX
#define HELP_TEXT    \
"\njavaldx is necessary to make Java work on some UNIX platforms." \
"It prints a string to std out that consists of directories which " \
"have to be included into the DYLD_LIBRARY_PATH variable.The setting of " \
"the variable usually occurs in a shell script that runs javaldx.\n" \
"The directories are from the chosen java installation. \n" \
"Options are: \n"\
"--help or -h\n"
#else
#define HELP_TEXT    \
"\njavaldx is necessary to make Java work on some UNIX platforms." \
"It prints a string to std out that consists of directories which " \
"have to be included into the LD_LIBRARY_PATH variable.The setting of " \
"the variable usually occurs in a shell script that runs javaldx.\n" \
"The directories are from the chosen java installation. \n" \
"Options are: \n"\
"--help or -h\n"
#endif

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

    if (errcode == JFW_E_INVALID_SETTINGS)
    {
        fprintf(stderr,"javaldx failed. User must select a JRE from options dialog!");
        return -1;
    }
    else if (errcode != JFW_E_NONE)
    {
        fprintf(stderr,"javaldx failed! \n");
        return -1;
    }

    if (pInfo == NULL)
    {
        errcode = jfw_findAndSelectJRE( & pInfo);
        if (errcode == JFW_E_NO_JAVA_FOUND)
        {
            fprintf(stderr,"javaldx: Could not find a Java Runtime Environment! \n");
            return 0;
        }
        else if (errcode != JFW_E_NONE && errcode != JFW_E_DIRECT_MODE)
        {
            fprintf(stderr,"javaldx failed!\n");
            return -1;
        }
    }

    //Only do something if the sunjavaplugin created this JavaInfo
    rtl::OUString sVendor1(RTL_CONSTASCII_USTRINGPARAM("Sun Microsystems Inc."));
    rtl::OUString sVendor2(RTL_CONSTASCII_USTRINGPARAM("IBM Corporation"));
    rtl::OUString sVendor3(RTL_CONSTASCII_USTRINGPARAM("Blackdown Java-Linux Team"));
    rtl::OUString sVendor4(RTL_CONSTASCII_USTRINGPARAM("Apple Inc."));
    rtl::OUString sVendor5(RTL_CONSTASCII_USTRINGPARAM("Apple Computer, Inc."));
    rtl::OUString sVendor6(RTL_CONSTASCII_USTRINGPARAM("BEA Systems, Inc."));
    rtl::OUString sVendor7(RTL_CONSTASCII_USTRINGPARAM("Free Software Foundation, Inc."));
    if ( ! (sVendor1.equals(pInfo->sVendor) == sal_True
            || sVendor2.equals(pInfo->sVendor) == sal_True
            || sVendor3.equals(pInfo->sVendor) == sal_True
            || sVendor4.equals(pInfo->sVendor) == sal_True
            || sVendor5.equals(pInfo->sVendor) == sal_True
            || sVendor6.equals(pInfo->sVendor) == sal_True
        || sVendor7.equals(pInfo->sVendor) == sal_True))
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





