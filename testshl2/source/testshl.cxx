/*************************************************************************
 *
 *  $RCSfile: testshl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:36:30 $
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
#include <unistd.h>

#ifdef SOLARIS
#include <sys/time.h>
#endif

#ifdef WNT
#define UNDER_WINDOWS_DEBUGGING
// Nice feature, to debug under windows, install msdev locally and use DebugBreak() to stop a new process at a point you want.
#ifdef UNDER_WINDOWS_DEBUGGING
#include <tools/presys.h>
#include <windows.h>
#include <MAPIWin.h>
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

void my_sleep(int sec)
{
#ifdef WNT
            Sleep(sec * 1000);
#else
            usleep(sec * 1000000); // 10 ms
#endif
}

// -----------------------------------------------------------------------------
class ProcessHandler
{
    std::string m_sProcessIDFilename;
    void write(int);
    int  getPID();
public:
    ProcessHandler();
    void setName(std::string const& _sFilename);
    ~ProcessHandler();
};

// ------------------------------- ProcessHelper -------------------------------
ProcessHandler::ProcessHandler() {}

void ProcessHandler::setName(std::string const& _sPIDFilename)
{
    m_sProcessIDFilename = _sPIDFilename;
    write(getPID());
}

int ProcessHandler::getPID()
{
    int nPID = 0;
#ifdef WNT
    nPID = GetCurrentProcessId();
#else
    nPID = getpid();
#endif

    return nPID;
}

void ProcessHandler::write(int _nPID)
{
    // get own PID

    if (m_sProcessIDFilename.size() > 0)
    {
        FILE* out;
        out = fopen(m_sProcessIDFilename.c_str(), "w");
        if (!out)
        {
            fprintf(stderr, "warning: (testshl.cxx) can't write own pid.\n");
            return;
            // exit(0);
        }
        fprintf(out, "%d", _nPID);
        fclose(out);
        my_sleep(2); // short wait, so testshl2_timeout can find this pid.
    }
    else
    {
        fprintf(stderr, "warning: (testshl.cxx) PID Filename empty, must set.\n");
    }
}

ProcessHandler::~ProcessHandler()
{
    if (m_sProcessIDFilename.size() > 0)
    {
        write(0);
        my_sleep(2);
        fprintf(stderr, "hint: it is save to remove PID file.\n");
    }
}

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
        "-dnrmsf,       do not remove signalfile. ",
        "-jobonly=s,    job control file, these jobs should only executed.",
        "-jobfilter=s,  use a filter for just some tests.",
        "-onlyshowjobs, show only all jobs, do no tests.",
        "-forward=s,    this string will forwarded to the test libraries.",
        "-projectid=s,  this text is added to the date output line.",
        "-buildid=s,    this text is added to the date output line.",
        "-waitforkey,   wait until key pressed.",
        "-verbose,      be verbose.",
        "-pid=s,        write current process id to file",
        "-endless,      testshl runs endless, for test only!!!",
        "-h:s,          display help or help on option",
        "-help:s,       see -h",
        NULL
    };

    // rtl::OString* pTest = new rtl::OString("test");

    ProcessHandler aCurrentProcess;

    GetOpt opt( argv, optionSet );
    if ( opt.hasOpt("-pid") )
    {
        aCurrentProcess.setName(opt.getOpt("-pid").getStr());
    }

    if ( opt.hasOpt("-verbose") )
    {
        fprintf(stderr, "testshl2 $Revision: 1.11 $\n");
    }

    // someone indicates that he needs help
    if ( opt.hasOpt( "-h" ) || opt.hasOpt( "-help" ) )
    {
        opt.showUsage();
        exit(0);
    }

    if ( opt.hasOpt("-endless"))
    {
        while(1)
        {
            my_sleep(1);
        }
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

//#ifdef UNDER_WINDOWS_DEBUGGING
//    DebugBreak();
//#endif

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

    // this is a debug extension, so you can read the output and after a key is pressed the program will end.
    if (opt.hasOpt("-waitforkey"))
    {
        fprintf(stderr, "Press return key.");
        fflush(stderr);
        getchar();
    }

    return 0;
}

