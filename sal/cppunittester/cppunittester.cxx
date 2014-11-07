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

#ifdef WNT
#include <windows.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <sal/types.h>
#include "cppunittester/protectorfactory.hxx"
#include "osl/module.h"
#include "osl/module.hxx"
#include "osl/thread.h"
#include "rtl/process.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/ustring.hxx"
#include "sal/main.h"

#include "cppunit/CompilerOutputter.h"
#include "cppunit/Exception.h"
#include "cppunit/TestFailure.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"
#include "cppunit/TestRunner.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/plugin/PlugInManager.h"
#include "cppunit/plugin/DynamicLibraryManagerException.h"
#include "cppunit/portability/Stream.h"

#include "boost/noncopyable.hpp"
#include <boost/scoped_array.hpp>
#include "boost/static_assert.hpp"

namespace {

void usageFailure() {
    std::cerr
        << ("Usage: cppunittester (--protector <shared-library-path>"
            " <function-symbol>)* <shared-library-path>")
        << std::endl;
    std::exit(EXIT_FAILURE);
}

rtl::OUString getArgument(sal_Int32 index) {
    rtl::OUString arg;
    rtl_getAppCommandArg(index, &arg.pData);
    return arg;
}

std::string convertLazy(rtl::OUString const & s16) {
    rtl::OString s8(rtl::OUStringToOString(s16, osl_getThreadTextEncoding()));
    BOOST_STATIC_ASSERT(sizeof (sal_Int32) <= sizeof (std::string::size_type));
        // ensure following cast is legitimate
    return std::string(
        s8.getStr(), static_cast< std::string::size_type >(s8.getLength()));
}

#if defined TIMETESTS
//Output how long each test took
class TimingListener
    : public CppUnit::TestListener
    , private boost::noncopyable
{
public:
    void startTest( CppUnit::Test *) SAL_OVERRIDE
    {
        m_nStartTime = osl_getGlobalTimer();
    }

    void endTest( CppUnit::Test *test ) SAL_OVERRIDE
    {
        sal_uInt32 nEndTime = osl_getGlobalTimer();
        std::cout << test->getName() << ": " << nEndTime-m_nStartTime
            << "ms" << std::endl;
    }

private:
    sal_uInt32 m_nStartTime;
};
#endif

#ifdef UNX
#include <stdlib.h>
// Setup an env variable so that temp file (or other) can
// have a useful value to identify the source
class EyecatcherListener
    : public CppUnit::TestListener
    , private boost::noncopyable
{
public:
    void startTest( CppUnit::Test* test) SAL_OVERRIDE
    {
        boost::scoped_array<char> tn(new char [ test->getName().length() + 2 ]);
        strcpy(tn.get(), test->getName().c_str());
        int len = strlen(tn.get());
        for(int i = 0; i < len; i++)
        {
            if(!isalnum(tn[i]))
            {
                tn[i] = '_';
            }
        }
        tn[len] = '_';
        tn[len + 1] = 0;
        setenv("LO_TESTNAME", tn.get(), true);
    }

    void endTest( CppUnit::Test* /* test */ ) SAL_OVERRIDE
    {
    }
};
#endif

class LogFailuresAsTheyHappen : public CppUnit::TestListener
{
public:
    virtual void addFailure( const CppUnit::TestFailure &failure ) SAL_OVERRIDE
    {
        printFailureLocation( failure.sourceLine() );
        printFailedTestName( failure );
        printFailureMessage( failure );
    }

private:
    void printFailureLocation( const CppUnit::SourceLine &sourceLine )
    {
        if ( !sourceLine.isValid() )
            std::cerr << "unknown:0:";
        else
            std::cerr << sourceLine.fileName() << ":" << sourceLine.lineNumber() << ":";
    }

    void printFailedTestName( const CppUnit::TestFailure &failure )
    {
        std::cerr << failure.failedTestName() << std::endl;
    }

    void printFailureMessage( const CppUnit::TestFailure &failure )
    {
        std::cerr << failure.thrownException()->message().shortDescription() << std::endl;
        std::cerr << failure.thrownException()->message().details() << std::endl;
    }
};

//Allow the whole uniting testing framework to be run inside a "Protector"
//which knows about uno exceptions, so it can print the content of the
//exception before falling over and dying
class CPPUNIT_API ProtectedFixtureFunctor
    : public CppUnit::Functor
    , private boost::noncopyable
{
private:
    const std::string &testlib;
    const std::string &args;
    std::vector<CppUnit::Protector *> &protectors;
    CppUnit::TestResult &result;
public:
    ProtectedFixtureFunctor(const std::string& testlib_, const std::string &args_, std::vector<CppUnit::Protector*> &protectors_, CppUnit::TestResult &result_)
        : testlib(testlib_)
        , args(args_)
        , protectors(protectors_)
        , result(result_)
    {
    }
    bool run() const
    {
#ifdef DISABLE_DYNLOADING

        // NOTE: Running cppunit unit tests on iOS was something I did
        // only very early (several years ago) when starting porting
        // this stuff to iOS. The complicated mechanisms to do build
        // such unit test single executables have surely largely
        // bit-rotted or been semi-intentionally broken since. This
        // stuff here left for information only. --tml 2014.

        // For iOS cppunit plugins aren't really "plugins" (shared
        // libraries), but just static archives. In the real main
        // program of a cppunit app, which calls the lo_main() that
        // the SAL_IMPLEMENT_MAIN() below expands to, we specifically
        // call the initialize methods of the CppUnitTestPlugIns that
        // we statically link to the app executable.
#else
        CppUnit::PlugInManager manager;
        try {
            manager.load(testlib, args);
        } catch (const CppUnit::DynamicLibraryManagerException &e) {
            std::cerr << "DynamicLibraryManagerException: \"" << e.what() << "\"\n";
#ifdef WIN32
            const char *pPath = getenv ("PATH");
            if (pPath && strlen (pPath) > 256)
            {
                std::cerr << "Windows has significant build problems with long PATH variables ";
                std::cerr << "please check your PATH variable and re-autogen.\n";
            }
#endif
            std::cerr << "Path is '" << getenv("PATH") << "'\n";
            return false;
        }
#endif

        for (size_t i = 0; i < protectors.size(); ++i)
            result.pushProtector(protectors[i]);

        bool success;
        {
            CppUnit::TestResultCollector collector;
            result.addListener(&collector);

            LogFailuresAsTheyHappen logger;
            result.addListener(&logger);

#ifdef TIMETESTS
            TimingListener timer;
            result.addListener(&timer);
#endif

#ifdef UNX
            EyecatcherListener eye;
            result.addListener(&eye);
            // set this to track down files created before first test method
            std::string lib(testlib.substr(testlib.rfind('/')+1));
            setenv("LO_TESTNAME", lib.c_str(), true);
#endif

            CppUnit::TestRunner runner;
            runner.addTest(
                CppUnit::TestFactoryRegistry::getRegistry().makeTest());
            runner.run(result);

            CppUnit::CompilerOutputter outputter(&collector, CppUnit::stdCErr());
            outputter.setNoWrap();
            outputter.write();
            success = collector.wasSuccessful();
        }

        for (size_t i = 0; i < protectors.size(); ++i)
            result.popProtector();

        return success;
    }
    virtual bool operator()() const SAL_OVERRIDE
    {
        return run();
    }
};

}

SAL_IMPLEMENT_MAIN() {
#ifdef WNT
    //Disable Dr-Watson in order to crash simply without popup dialogs under
    //windows
    DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetErrorMode(SEM_NOGPFAULTERRORBOX|dwMode);
#ifdef _DEBUG // These functions are present only in the debgging runtime
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
#endif

    std::vector<CppUnit::Protector *> protectors;
    CppUnit::TestResult result;
    std::string args;
    std::string testlib;
    sal_uInt32 index = 0;
    while (index < rtl_getAppCommandArgCount())
    {
        rtl::OUString arg = getArgument(index);
        if ( arg != "--protector" )
        {
            if (testlib.empty())
            {
                testlib = rtl::OUStringToOString(arg, osl_getThreadTextEncoding()).getStr();
                args += testlib;
            }
            else
            {
                args += ' ';
                args += rtl::OUStringToOString(arg, osl_getThreadTextEncoding()).getStr();
            }
            ++index;
            continue;
        }
        if (rtl_getAppCommandArgCount() - index < 3) {
            usageFailure();
        }
        rtl::OUString lib(getArgument(index + 1));
        rtl::OUString sym(getArgument(index + 2));
#ifndef DISABLE_DYNLOADING
        osl::Module mod(lib, SAL_LOADMODULE_GLOBAL);
        oslGenericFunction fn = mod.getFunctionSymbol(sym);
        mod.release();
#else
        oslGenericFunction fn = 0;
        if (sym == "unoexceptionprotector")
            fn = (oslGenericFunction) unoexceptionprotector;
        else if (sym == "unobootstrapprotector")
            fn = (oslGenericFunction) unobootstrapprotector;
        else if (sym == "vclbootstrapprotector")
            fn = (oslGenericFunction) vclbootstrapprotector;
        else
        {
            std::cerr
                << "Only unoexceptionprotector or unobootstrapprotector protectors allowed"
                << std::endl;
            std::exit(EXIT_FAILURE);
        }
#endif
        CppUnit::Protector *protector = fn == 0
            ? 0
            : (*reinterpret_cast< cppunittester::ProtectorFactory * >(fn))();
        if (protector == 0) {
            std::cerr
                << "Failure instantiating protector \"" << convertLazy(lib)
                << "\", \"" << convertLazy(sym) << '"' << std::endl;
            std::exit(EXIT_FAILURE);
        }
        protectors.push_back(protector);
        index+=3;
    }

    ProtectedFixtureFunctor tests(testlib, args, protectors, result);
    bool ok = tests.run();

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
