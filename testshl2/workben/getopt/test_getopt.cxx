/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_getopt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 10:17:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

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

