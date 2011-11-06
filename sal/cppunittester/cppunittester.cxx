/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "precompiled_sal.hxx"
#include "sal/config.h"

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

#include "preextstl.h"
#include "cppunit/CompilerOutputter.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"
#include "cppunit/TestRunner.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/plugin/PlugInManager.h"
#include "cppunit/portability/Stream.h"
#include "cppunit/plugin/DynamicLibraryManagerException.h"
#include "postextstl.h"

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
          > std::numeric_limits< std::string::size_type >::max())
         ? std::numeric_limits< std::string::size_type >::max()
         : static_cast< std::string::size_type >(s8.getLength())));
}

std::string convertStrict(rtl::OUString const & s16) {
    rtl::OString s8;
    if (!s16.convertToString(
            &s8, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
        || (static_cast< sal_uInt32 >(s8.getLength())
            > std::numeric_limits< std::string::size_type >::max()))
    {
        std::cerr
            << "Failure converting argument from UTF-16 back to system encoding"
            << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return std::string(
        s8.getStr(), static_cast< std::string::size_type >(s8.getLength()));
}

}

SAL_IMPLEMENT_MAIN() {
    CppUnit::TestResult result;
    sal_uInt32 index = 0;
    for (; index < rtl_getAppCommandArgCount(); index += 3) {
        if (!getArgument(index).equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("--protector")))
        {
            break;
        }
        if (rtl_getAppCommandArgCount() - index < 3) {
            usageFailure();
        }
        rtl::OUString lib(getArgument(index + 1));
        rtl::OUString sym(getArgument(index + 2));
        oslGenericFunction fn = (new osl::Module(lib, SAL_LOADMODULE_GLOBAL))
            ->getFunctionSymbol(sym);
        CppUnit::Protector * p = fn == 0
            ? 0
            : (*reinterpret_cast< cppunittester::ProtectorFactory * >(fn))();
        if (p == 0) {
            std::cerr
                << "Failure instantiating protector \"" << convertLazy(lib)
                << "\", \"" << convertLazy(sym) << '"' << std::endl;
            std::exit(EXIT_FAILURE);
        }
        result.pushProtector(p);
    }
    if (rtl_getAppCommandArgCount() - index != 1) {
        usageFailure();
    }

    bool bSuccess = false;
    try {
        CppUnit::PlugInManager manager;
        manager.load(convertStrict(getArgument(index)));
        CppUnit::TestRunner runner;
        runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
        CppUnit::TestResultCollector collector;
        result.addListener(&collector);
        runner.run(result);
        CppUnit::CompilerOutputter(&collector, CppUnit::stdCErr()).write();
        bSuccess = collector.wasSuccessful();
    } catch( CppUnit::DynamicLibraryManagerException& e) {
        std::cerr << "DynamicLibraryManagerException: \"" << e.what() << "\"\n";
    }

    return bSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
