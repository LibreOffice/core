/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "precompiled_sal.hxx"
#include "sal/config.h"
#include "sal/precppunit.hxx"

#ifdef WNT
#include <windows.h>
#endif

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
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
#include "sal/types.h"

#include "cppunit/CompilerOutputter.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"
#include "cppunit/TestRunner.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/plugin/PlugInManager.h"
#include "cppunit/portability/Stream.h"

#include "boost/noncopyable.hpp"

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
    return std::string(
        s8.getStr(),
        ((static_cast< sal_uInt32 >(s8.getLength())
          > (std::numeric_limits< std::string::size_type >::max)())
         ? (std::numeric_limits< std::string::size_type >::max)()
         : static_cast< std::string::size_type >(s8.getLength())));
}

//Allow the whole uniting testing framework to be run inside a "Protector"
//which knows about uno exceptions, so it can print the content of the
//exception before falling over and dying
class CPPUNIT_API ProtectedFixtureFunctor : public CppUnit::Functor, private boost::noncopyable
{
private:
    const std::string &testlib;
    const std::string &args;
    CppUnit::TestResult &result;
public:
    ProtectedFixtureFunctor(const std::string& testlib_, const std::string &args_, CppUnit::TestResult &result_)
        : testlib(testlib_)
        , args(args_)
        , result(result_)
    {
    }
    bool run() const
    {
        CppUnit::PlugInManager manager;
        manager.load(testlib, args);
        CppUnit::TestRunner runner;
        runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
        CppUnit::TestResultCollector collector;
        result.addListener(&collector);
        runner.run(result);
        CppUnit::CompilerOutputter(&collector, CppUnit::stdCErr()).write();
        return collector.wasSuccessful();
    }
    virtual bool operator()() const
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

    CppUnit::TestResult result;
    cppunittester::LibreOfficeProtector *throw_protector = 0;
    std::string args;
    std::string testlib;
    sal_uInt32 index = 0;
    while (index < rtl_getAppCommandArgCount())
    {
        rtl::OUString arg = getArgument(index);
        if (!arg.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("--protector")))
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
        oslGenericFunction fn = (new osl::Module(lib, SAL_LOADMODULE_GLOBAL))
            ->getFunctionSymbol(sym);
        throw_protector = fn == 0
            ? 0
            : (*reinterpret_cast< cppunittester::ProtectorFactory * >(fn))();
        if (throw_protector == 0) {
            std::cerr
                << "Failure instantiating protector \"" << convertLazy(lib)
                << "\", \"" << convertLazy(sym) << '"' << std::endl;
            std::exit(EXIT_FAILURE);
        }
        result.pushProtector(throw_protector);
        index+=3;
    }

    bool ok = false;
    ProtectedFixtureFunctor tests(testlib, args, result);
    //if the unoprotector was given on the command line, use it to catch
    //and report the error message of exceptions
    if (throw_protector)
        ok = throw_protector->protect(tests);
    else
        ok = tests.run();

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
