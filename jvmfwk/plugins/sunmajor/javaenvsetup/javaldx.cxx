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

#include <sal/config.h>

#include <memory>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sal/main.h>
#include <sal/types.h>
#include <osl/thread.h>
#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>
#include <jvmfwk/framework.hxx>
#include <o3tl/string_view.hxx>


static bool hasOption(char const * szOption, int argc, char** argv);
static OString getLD_LIBRARY_PATH(const rtl::ByteSequence & vendorData);
static bool findAndSelect(std::unique_ptr<JavaInfo>*);

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
    try
    {
        if( hasOption("--help",argc, argv) || hasOption("-h", argc, argv))
        {
            fprintf(stdout, HELP_TEXT);// default
            return 0;
        }
        bool bEnabled = false;
        javaFrameworkError errcode = jfw_getEnabled( & bEnabled);
        if (errcode == JFW_E_NONE && !bEnabled)
        {
                //Do not do any preparation because that may only slow startup time.
            fprintf(stdout, "\n");
            return 0;
        }
        else if (errcode != JFW_E_NONE && errcode != JFW_E_DIRECT_MODE)
        {
            SAL_WARN("jvmfwk.javaenvsetup", "javaldx failed!");
            return -1;
        }

        std::unique_ptr<JavaInfo> aInfo;
        errcode = jfw_getSelectedJRE(&aInfo);

        if (errcode != JFW_E_NONE && errcode != JFW_E_INVALID_SETTINGS)
        {
            SAL_WARN("jvmfwk.javaenvsetup", "javaldx failed!");
            return -1;
        }

        if (!aInfo)
        {
            if (!findAndSelect(&aInfo))
                return -1;
        }
        else
        {
            //check if the JRE was not uninstalled
            bool bExist = false;
            errcode = jfw_existJRE(aInfo.get(), &bExist);
            if (errcode == JFW_E_NONE)
            {
                if (!bExist && !findAndSelect(&aInfo))
                    return -1;
            }
            else
            {
                SAL_WARN("jvmfwk.javaenvsetup", "javaldx: Could not determine if JRE still exist");
                return -1;
            }
        }

        OString sPaths = getLD_LIBRARY_PATH(aInfo->arVendorData);
        fprintf(stdout, "%s\n", sPaths.getStr());
    }
    catch (const std::exception& e)
    {
        std::cerr << "javaldx failed! " << e.what() << std::endl;
        return -1;
    }

    return 0;
}

OString getLD_LIBRARY_PATH(const rtl::ByteSequence & vendorData)
{
    const sal_Unicode* chars = reinterpret_cast<sal_Unicode const *>(vendorData.getConstArray());
    sal_Int32 len = vendorData.getLength();
    OUString sData(chars, len / 2);
    //the runtime lib is on the first line
    sal_Int32 index = 0;
    std::u16string_view aToken = o3tl::getToken(sData, 1, '\n', index);

    OString paths =
        OUStringToOString(aToken, osl_getThreadTextEncoding());
    return paths;
}

static bool hasOption(char const * szOption, int argc, char** argv)
{
    bool retVal= false;
    for(int i= 1; i < argc; i++)
    {
        if( ! strcmp(argv[i], szOption))
        {
            retVal= true;
            break;
        }
    }
    return retVal;
}

static bool findAndSelect(std::unique_ptr<JavaInfo> * ppInfo)
{
    javaFrameworkError errcode = jfw_findAndSelectJRE(ppInfo);
    if (errcode == JFW_E_NO_JAVA_FOUND)
    {
        SAL_WARN("jvmfwk.javaenvsetup", "javaldx: Could not find a Java Runtime Environment!");
        return false;
    }
    else if (errcode != JFW_E_NONE && errcode != JFW_E_DIRECT_MODE)
    {
        SAL_WARN("jvmfwk.javaenvsetup", "javaldx failed!");
        return false;
    }
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
