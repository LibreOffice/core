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
#include "precompiled_testshl2.hxx"

#include <stdio.h>
#include <stdlib.h>

#ifdef SOLARIS
#include <sys/time.h>
#endif

#ifdef WNT
// #define UNDER_WINDOWS_DEBUGGING
// Nice feature, to debug under windows, install msdev locally and use DebugBreak() to stop a new process at a point you want.
#ifdef UNDER_WINDOWS_DEBUGGING
#include <tools/presys.h>
#include <windows.h>
#include <tools/postsys.h>

#define VCL_NEED_BASETSD

#endif /* UNDER_WINDOWS_DEBUGGING */
#endif /* WNT */

#include <iostream>
#include <vector>
#include <rtl/string.hxx>

#include "filehelper.hxx"
#include "getopt.hxx"

// #include <osl/time.h>

using namespace std;

// -----------------------------------------------------------------------------

/**
 * display usage screen
 */

void usage()
{
    fprintf( stdout,
            "USAGE: testshl shlname [-boom][-verbose][-log][-his][-msg]\n" );
    exit(0);
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    (void) argc;
    static char const* optionSet[] = {
        "-boom,     stop near error position, exception only",
        "-mode=s,   the output mode, emacs, xml, old. Default is -mode old",
        "-logPath=s, destination path for logging",
        "-tc=s@,    name(s) of testcase(s) to generate",
        "-h:s,      display help or help on option",
        "-help:s,   see -h",
        NULL
    };

    GetOpt opt( argv, optionSet );

    // someone indicates that he needs help
    if ( opt.hasOpt( "-h" ) || opt.hasOpt( "-help" ) )
    {
        opt.showUsage();
        // exit(0);
    }

    // get path for logging stuff..
    rtl::OString logPth;
    // ...if available
    if ( opt.hasOpt( "-logPth" ))
    {
        logPth = opt.getOpt( "-logPth" );
    }

    rtl::OString param;
    if ( ! opt.getParams().empty() ) {
        param = opt.getFirstParam();

        std::cout << "all non '-' Parameter values" << std::endl;

        vector<rtl::OString>& aVec = opt.getParams();
        for(vector<rtl::OString>::const_iterator it = aVec.begin();
            it != aVec.end();
            ++it)
        {
            std::cout << (*it).getStr() << std::endl;
        }
    }

    if ( opt.hasOpt("-tc"))
    {
        std::cout << "Parameter -tc" << std::endl;
        vector<rtl::OString>& aVec = opt.getOptVec( "-tc" );

        for(vector<rtl::OString>::const_iterator it = aVec.begin();
            it != aVec.end();
            ++it)
        {
            std::cout << (*it).getStr() << std::endl;
        }
    }

    return 0;
}

