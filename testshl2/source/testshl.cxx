/*************************************************************************
 *
 *  $RCSfile: testshl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: lla $ $Date: 2003-03-05 08:44:06 $
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
#include <memory> /* auto_ptr */
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
std::auto_ptr<CppUnit::TestResult> initResult(GetOpt & _aOptions)
{
    std::auto_ptr<CppUnit::TestResult> pResult;
    if (_aOptions.hasOpt("-mode"))
    {
        if (_aOptions.getOpt("-mode").equals("emacs") == sal_True)
        {
            pResult = std::auto_ptr<CppUnit::TestResult>(new CppUnit::emacsTestResult(_aOptions));
        }
    }
    else
    {
        pResult = std::auto_ptr<CppUnit::TestResult>(new CppUnit::testshlTestResult(_aOptions));
    }
    return pResult;
}

// -----------------------------------------------------------------------------

std::auto_ptr<Outputter> initOutputter(GetOpt & _aOptions)
{
    std::auto_ptr<Outputter> pOutputter;

    if (_aOptions.hasOpt("-log"))
    {
        rtl::OString sLogFile = _aOptions.getOpt("-log");

        Log* pLog = new Log( FileHelper::convertPath( sLogFile ) );
        if ( pLog->open() != osl::FileBase::E_None )
        {
            // cerr << "could not open LogFile: " << pLog->getName().getStr() << endl;
            fprintf(stderr, "could not open LogFile: %s\n", pLog->getName().getStr());
            exit(1);
        }
        pOutputter = std::auto_ptr<Outputter>(new Outputter(pLog));
    }
    else
    {
        pOutputter = std::auto_ptr<Outputter>(new Outputter(std::cout));
    }

    return pOutputter;
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
        "-log=s,        destination file for logging",
        "-noerrors,     shows all tests, but not it's errors.",
        "-onlyerrors,   shows only the failed test functions",
/*      "-tc=s@,        name(s) of testcase(s) to generate", */
        "-sf=s,         absolute path and name to the signal file.",
//! new (helpful if at debug time)
        "-dntsf,        if given, do not touch signal file, also if one exist.",
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

    bool bLibrary = true;
    if (opt.getParams().empty())
    {
        // no library is given, but if a jobonly list is given, we should generate UNKNOWN errors.
        if (! opt.hasOpt("-jobonly"))
        {
            // std::cerr << "error: At least a library or a job file should given." << std::endl;
            fprintf(stderr, "error: At least a library or a job file should given.\n");
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

    AutomaticRegisterHelper aHelper(suLibraryName, opt /*, &aJobs*/);

    // create a TestResult
    std::auto_ptr<CppUnit::TestResult> pResult = initResult(opt);

    // call all test functions
    aHelper.CallAll(pResult.get());

    // create and open log
    std::auto_ptr<Outputter> pOutput = initOutputter(opt);

    // give the output
    pResult->print(*pOutput.get());
    return 0;
}

// all functions, which exist in the library
// all functions + the given, no one twice
// only the given functions

// Exception info
// Due to the fact that exceptions will caught, on exceptions the ASSERTION text is given.
// but we know also the real thrown exception

// PARAMETER:
// - emacs mode
// - test runner in perl.
// - Version/Buildno. in Date
// - xml

// Remove rtl_tres_state from SAL!
// no longer need, because
// the new tool use c_rtl_tres_state()


// What is a good test?
// init your data
// ASSERT( check if a generated result is, what you expect )
//
// that's all
// really all, no
// because who called this test.
// so we envelop it into a function.
// Same checks should envelop in a class
// but the question is also here, who create this class and called all it's containing functions?
// class
// {
// public:
//   test function1
//   test function2
// };
// Due to the fact, that there exist no official way to get a list of all functions at run time,
// we have to register the functions by hand.
// To cut down the overhead,
// there exist some macros to help you to register your functions into an internal registry.
// class
// {
// public:
//   test function1
//   test function2
//   REGISTER(test functions)
// };

// all classes have also to register there exist also a macro
// NAMED_REGISTER
// that's all
// too much?
// Ok, due to the fact that much code here is recur, there exist a code generator which help you to generate the
// code. You have to build a simple job list, which says, package.class.function for every entry.
// For every line in the job list there will create a function with an assertion stub.
// the code is compileable, you don't need to register the functions, because this code will also created for you.
// ok, you also need an addition to a makefile.mk which the code generator also build for you.
//
// So your work is reduced to write down a job list, call the code generator with this job list.
// If a makefile exist, there exist a makefile.new which contains the code to build the new generated stubs.
// So you have to replace the assertion stub() by your test code.
// Build your test library by calling dmake
// call testshl2 with this library.
// get info what works, what not.

// By a human readable line by line info.

// http://foldoc.doc.ic.ac.uk/foldoc/foldoc.cgi?automated+testing
// http://foldoc.doc.ic.ac.uk/foldoc/foldoc.cgi?fault-based+testing
// http://foldoc.doc.ic.ac.uk/foldoc/foldoc.cgi?confidence+test

