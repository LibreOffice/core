/*************************************************************************
 *
 *  $RCSfile: test_getopt.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2003-01-09 11:06:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    static char* optionSet[] = {
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

