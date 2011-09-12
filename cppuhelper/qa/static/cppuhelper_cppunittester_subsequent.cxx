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

#include "sal/config.h"
#include "sal/precppunit.hxx"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include "protectorfactory.hxx"
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
#include "cppunit/plugin/TestPlugIn.h"
#include "cppunit/plugin/PlugInParameters.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
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
    const std::string &args;
    CppUnit::TestResult &result;
public:
    ProtectedFixtureFunctor(const std::string &args_, CppUnit::TestResult &result_)
        : args(args_)
        , result(result_)
    {
    }
    bool run() const
    {
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

extern "C" CppUnitTestPlugIn
            *cppunitTest_cppuhelper_propertysetmixin(void);


SAL_IMPLEMENT_MAIN() {
    TestPlugInSignature plugs[] = {
        cppunitTest_cppuhelper_propertysetmixin,
        NULL
    };
    CppUnit::TestResult result;
    std::string args;
    bool ok = false;
    for (TestPlugInSignature *plug = plugs; *plug != NULL; plug++) {
        CppUnitTestPlugIn *iface;
        iface = (*plug)();
        iface->initialize(&CppUnit::TestFactoryRegistry::getRegistry(), CppUnit::PlugInParameters());
    }
    ProtectedFixtureFunctor tests(args, result);
    ok = tests.run();

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
