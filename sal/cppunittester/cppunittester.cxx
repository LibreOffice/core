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

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef UNX
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <cstdlib>
#include <iostream>
#include <string>
#include <sal/log.hxx>
#include <sal/types.h>
#include <cppunittester/protectorfactory.hxx>
#include <osl/module.h>
#include <osl/module.hxx>
#include <osl/thread.h>
#include <rtl/character.hxx>
#include <rtl/process.h>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <rtl/textcvt.h>
#include <rtl/ustring.hxx>
#include <sal/main.h>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/Exception.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/plugin/PlugInManager.h>
#include <cppunit/plugin/DynamicLibraryManagerException.h>
#include <cppunit/portability/Stream.h>

#include <memory>
#include <boost/algorithm/string.hpp>

#include <algorithm>

namespace {

void usageFailure() {
    std::cerr
        << ("Usage: cppunittester (--protector <shared-library-path>"
            " <function-symbol>)* <shared-library-path>")
        << std::endl;
    std::exit(EXIT_FAILURE);
}

OUString getArgument(sal_Int32 index) {
    OUString arg;
    osl_getCommandArg(index, &arg.pData);
    return arg;
}

std::string convertLazy(OUString const & s16) {
    OString s8(OUStringToOString(s16, osl_getThreadTextEncoding()));
    static_assert(sizeof (sal_Int32) <= sizeof (std::string::size_type), "must be at least the same size");
        // ensure following cast is legitimate
    return std::string(
        s8.getStr(), static_cast< std::string::size_type >(s8.getLength()));
}

//Output how long each test took
class TimingListener
    : public CppUnit::TestListener
{
public:
    TimingListener()
        : m_nStartTime(0)
    {
    }
    TimingListener(const TimingListener&) = delete;
    TimingListener& operator=(const TimingListener&) = delete;

    void startTest( CppUnit::Test *) override
    {
        m_nStartTime = osl_getGlobalTimer();
    }

    void endTest( CppUnit::Test *test ) override
    {
        sal_uInt32 nEndTime = osl_getGlobalTimer();
        std::cout << test->getName() << " finished in: "
            << nEndTime-m_nStartTime << "ms" << std::endl;
    }

private:
    sal_uInt32 m_nStartTime;
};

#ifdef UNX
#include <stdlib.h>
// Setup an env variable so that temp file (or other) can
// have a useful value to identify the source
class EyecatcherListener
    : public CppUnit::TestListener
{
public:
    EyecatcherListener() = default;
    EyecatcherListener(const EyecatcherListener&) = delete;
    EyecatcherListener& operator=(const EyecatcherListener&) = delete;
    void startTest( CppUnit::Test* test) override
    {
        std::unique_ptr<char[]> tn(new char [ test->getName().length() + 2 ]);
        strcpy(tn.get(), test->getName().c_str());
        int len = strlen(tn.get());
        for(int i = 0; i < len; i++)
        {
            if(!rtl::isAsciiAlphanumeric(static_cast<unsigned char>(tn[i])))
            {
                tn[i] = '_';
            }
        }
        tn[len] = '_';
        tn[len + 1] = 0;
        setenv("LO_TESTNAME", tn.get(), true);
    }

    void endTest( CppUnit::Test* /* test */ ) override
    {
    }
};
#endif

class LogFailuresAsTheyHappen : public CppUnit::TestListener
{
public:
    virtual void addFailure( const CppUnit::TestFailure &failure ) override
    {
        printFailureLocation( failure.sourceLine() );
        printFailedTestName( failure );
        printFailureMessage( failure );
    }

private:
    static void printFailureLocation( const CppUnit::SourceLine &sourceLine )
    {
        if ( !sourceLine.isValid() )
            std::cerr << "unknown:0:";
        else
            std::cerr << sourceLine.fileName() << ":" << sourceLine.lineNumber() << ":";
    }

    static void printFailedTestName( const CppUnit::TestFailure &failure )
    {
        std::cerr << failure.failedTestName() << std::endl;
    }

    static void printFailureMessage( const CppUnit::TestFailure &failure )
    {
        std::cerr << failure.thrownException()->message().shortDescription() << std::endl;
        std::cerr << failure.thrownException()->message().details() << std::endl;
    }
};

struct test_name_compare
{
    explicit test_name_compare(const std::string& rName):
        maName(rName)
    {
    }

    bool operator()(const std::string& rCmp)
    {
        size_t nPos = maName.find(rCmp);
        if (nPos == std::string::npos)
            return false;

        size_t nEndPos = nPos + rCmp.size();
        return nEndPos == maName.size();
    }

    std::string maName;
};

bool addRecursiveTests(const std::vector<std::string>& test_names, CppUnit::Test* pTest, CppUnit::TestRunner& rRunner)
{
    bool ret(false);
    for (int i = 0; i < pTest->getChildTestCount(); ++i)
    {
        CppUnit::Test* pNewTest = pTest->getChildTestAt(i);
        ret |= addRecursiveTests(test_names, pNewTest, rRunner);
        if (std::any_of(test_names.begin(), test_names.end(), test_name_compare(pNewTest->getName())))
        {
            rRunner.addTest(pNewTest);
            ret = true;
        }
    }
    return ret;
}

//Allow the whole uniting testing framework to be run inside a "Protector"
//which knows about uno exceptions, so it can print the content of the
//exception before falling over and dying
class CPPUNIT_API ProtectedFixtureFunctor
    : public CppUnit::Functor
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
    ProtectedFixtureFunctor(const ProtectedFixtureFunctor&) = delete;
    ProtectedFixtureFunctor& operator=(const ProtectedFixtureFunctor&) = delete;
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
        // The PlugInManager instance is deliberately leaked, so that the dynamic libraries it loads
        // are never unloaded (which could make e.g. pointers from other libraries' static data
        // structures to const data in those libraries, like some static OUString cache pointing at
        // a const OUStringLiteral, become dangling by the time those static data structures are
        // destroyed during exit):
        auto manager = new CppUnit::PlugInManager;
        try {
            manager->load(testlib, args);
        } catch (const CppUnit::DynamicLibraryManagerException &e) {
            std::cerr << "DynamicLibraryManagerException: \"" << e.what() << "\"\n";
#ifdef _WIN32
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

            TimingListener timer;
            result.addListener(&timer);

#ifdef UNX
            EyecatcherListener eye;
            result.addListener(&eye);
            // set this to track down files created before first test method
            std::string lib(testlib.substr(testlib.rfind('/')+1));
            setenv("LO_TESTNAME", lib.c_str(), true);
#endif

            const char* pVal = getenv("CPPUNIT_TEST_NAME");

            CppUnit::TestRunner runner;
            if (pVal)
            {
                std::vector<std::string> test_names;
                boost::split(test_names, pVal, boost::is_any_of("\t "));
                CppUnit::Test* pTest = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
                bool const added(addRecursiveTests(test_names, pTest, runner));
                if (!added)
                {
                    std::cerr << "\nFatal error: CPPUNIT_TEST_NAME contains no valid tests\n";
                    return false;
                }
            }
            else
            {
                runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
            }
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
    virtual bool operator()() const override
    {
        return run();
    }
};

#ifdef UNX

double get_time(timeval* time)
{
    double nTime = static_cast<double>(time->tv_sec);
    nTime += static_cast<double>(time->tv_usec)/1000000.0;
    return nTime;
}

OString generateTestName(const OUString& rPath)
{
    sal_Int32 nPathSep = rPath.lastIndexOf("/");
    OUString aTestName = rPath.copy(nPathSep+1);
    return OUStringToOString(aTestName, RTL_TEXTENCODING_UTF8);
}

void reportResourceUsage(const OUString& rPath)
{
    OUString aFullPath = rPath + ".resource.log";
    rusage resource_usage;
    getrusage(RUSAGE_SELF, &resource_usage);

    OString aPath = OUStringToOString(aFullPath, RTL_TEXTENCODING_UTF8);
    std::ofstream resource_file(aPath.getStr());
    resource_file << "Name = " << generateTestName(rPath) << std::endl;
    double nUserSpace = get_time(&resource_usage.ru_utime);
    double nKernelSpace = get_time(&resource_usage.ru_stime);
    resource_file << "UserSpace = " << nUserSpace << std::endl;
    resource_file << "KernelSpace = " << nKernelSpace << std::endl;
}
#else
void reportResourceUsage(const OUString& /*rPath*/)
{
}
#endif

}

SAL_IMPLEMENT_MAIN()
{
    bool ok = false;
    OUString path;
    try
    {
#ifdef _WIN32
        //Disable Dr-Watson in order to crash simply without popup dialogs under
        //windows
        DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
        SetErrorMode(SEM_NOGPFAULTERRORBOX|dwMode);
#ifdef _DEBUG // These functions are present only in the debugging runtime
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
        while (index < osl_getCommandArgCount())
        {
            OUString arg = getArgument(index);
            if (arg.startsWith("-env:CPPUNITTESTTARGET=", &path))
            {
                ++index;
                continue;
            }
            if (arg.startsWith("-env:"))
            {
                ++index;
                continue; // ignore it here - will be read later
            }
            if ( arg != "--protector" )
            {
                if (testlib.empty())
                {
                    testlib = OUStringToOString(arg, osl_getThreadTextEncoding()).getStr();
                    args += testlib;
                }
                else
                {
                    args += ' ';
                    args += OUStringToOString(arg, osl_getThreadTextEncoding()).getStr();
                }
                ++index;
                continue;
            }
            if (osl_getCommandArgCount() - index < 3) {
                usageFailure();
            }
            OUString lib(getArgument(index + 1));
            OUString sym(getArgument(index + 2));
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
            CppUnit::Protector *protector = fn == nullptr
                ? nullptr
                : (*reinterpret_cast< cppunittester::ProtectorFactory * >(fn))();
            if (protector == nullptr) {
                std::cerr
                    << "Failure instantiating protector \"" << convertLazy(lib)
                    << "\", \"" << convertLazy(sym) << '"' << std::endl;
                std::exit(EXIT_FAILURE);
            }
            protectors.push_back(protector);
            index+=3;
        }

        ProtectedFixtureFunctor tests(testlib, args, protectors, result);
        ok = tests.run();
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.app", "Fatal exception: " << e.what());
    }

    reportResourceUsage(path);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
