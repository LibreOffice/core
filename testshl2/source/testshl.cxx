/*************************************************************************
 *
 *  $RCSfile: testshl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: lla $ $Date: 2003-01-13 13:56:33 $
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

// #include <rtl/tres.h>
#include "autoregisterhelper.hxx"
// #include "cmdlinebits.hxx"
#include "log.hxx"
#include "filehelper.hxx"
#include "getopt.hxx"
#include "cppunit/result/outputter.hxx"
#include "cppunit/joblist.hxx"

// #include <osl/time.h>

using namespace std;

#include "cppunit/result/testshlTestResult.h"
#include "cppunit/result/emacsTestResult.hxx"

// Prototype for signal handling
void setSignalFilename(GetOpt & opt);

// -----------------------------------------------------------------------------

Log initLog(GetOpt & _aOptions)
{
    rtl::OString logPath;
    if (_aOptions.hasOpt("-logPath"))
    {
        logPath = _aOptions.getOpt("-logPath");
    }
    else
    {
        logPath = FileHelper::getTempPath().c_str();
    }
    rtl::OString sLogFile(logPath);
    sLogFile += "/testshl2.log";

    Log log( FileHelper::convertPath( sLogFile ) );
    if ( log.open() != osl::FileBase::E_None )
    {
        cerr << "could not open LogFile: " << log.getName().getStr() << endl;
        exit(1);
    }
    return log;
}



// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    static char* optionSet[] = {
        "-boom,         stop near error position, exception only",
        "-mode=s,       the output mode, emacs, xml, old. Default is -mode old",
        "-logPath=s,    destination path for logging",
        "-noerrors,     shows all tests, but not it's errors.",
        "-onlyerrors,   shows only the failed test functions",
/*      "-tc=s@,        name(s) of testcase(s) to generate", */
        "-sf=s,         absolute path and name to the signal file.",
        "-jobonly=s,    job control file, these jobs should only executed.",
        "-jobfilter=s,  use a filter for just some tests.",
        "-onlyshowjobs, show only all jobs, do no tests.",
        "-forward=s,    this string will forwarded to the test libraries.",
        "-h:s,          display help or help on option",
        "-help:s,       see -h",
        NULL
    };

    GetOpt opt( argv, optionSet );

    // someone indicates that he needs help
    if ( opt.hasOpt( "-h" ) || opt.hasOpt( "-help" ) )
    {
        opt.showUsage();
        exit(0);
    }

    // get path for logging stuff..
    rtl::OString logPth;
    // ...if available
    if ( opt.hasOpt( "-logPth" ))
    {
        logPth = opt.getOpt( "-logPth" );
    }

    bool bLibrary = true;
    if (opt.getParams().empty())
    {
        // no library is given, but if a jobonly list is given, we should generate UNKNOWN errors.
        if (! opt.hasOpt("-jobonly"))
        {
            std::cerr << "error: At least on library should given." << std::endl;
            opt.showUsage();
            exit(0);
        }
        else
        {
            bLibrary = false;
        }
    }

    setSignalFilename(opt);

    /* show usage screen if too less parameters */
    // if ( argc < 2 )
    //     usage();

#ifdef UNDER_WINDOWS_DEBUGGING
    DebugBreak();
#endif

    // ---
    //# CmdLineBits nCmdlinebitflags = createFlags( argc, argv );

    //# rtl::OUString suLibraryName = rtl::OUString::createFromAscii(argv[1]);
    rtl::OUString suLibraryName;
    if (bLibrary)
        suLibraryName = rtl::OStringToOUString(opt.getFirstParam(), RTL_TEXTENCODING_ASCII_US );

    // Log aLog = initLog(opt);
    AutomaticRegisterHelper aHelper(suLibraryName, opt /*, &aJobs*/);

    // create and open log
    CppUnit::TestResult* pResult;
    if (opt.hasOpt("-mode"))
    {
        if (opt.getOpt("-mode").equals("emacs") == sal_True)
        {
            pResult = new CppUnit::emacsTestResult(opt);
        }
    }
    else
    {
        pResult = new CppUnit::testshlTestResult(opt);
    }

    aHelper.CallAll(pResult);

    Outputter aOutput(cout);
    pResult->print(aOutput);
    // aLog << *pResult;

    // cout << (*pResult);
    // cout << "Done." << std::endl;

    delete pResult;
    return 0;
}

// all functions, which exist in the library
// all functions + the given, no one twice
// only the given functions

// Signals info
// We can only say which function has thrown a signal, and what signal was thrown,
// but not really where the signal was thrown. You have to debug this function by hand.

// Exception info
// Due to the fact that exceptions will caught, on exceptions the ASSERTION text is given.
// but we know also the real thrown exception

// PARAMETER:
// - BaselibTest.txt anders benennen.
// - in eigenes Verzeichnis setzen.
// - log file, nicht nur cout.
// - emacs mode
// - test runner in perl.
// - Version/Buildno. im Datum
// - xml

// Remove rtl_tres_state from SAL!
