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

#include <cstdlib>
#include <iostream>

#ifdef WNT
#include <windows.h>
#endif

#include "cppunit/CompilerOutputter.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"
#include "cppunit/TestRunner.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/plugin/PlugInManager.h"
#include "cppunit/portability/Stream.h"
#include "osl/thread.h"
#include "rtl/process.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/main.h"

SAL_IMPLEMENT_MAIN()
{

#ifdef WNT
    //Disable Dr-Watson in order to crash simply without popup dialogs under
    //windows
    DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetErrorMode(SEM_NOGPFAULTERRORBOX|dwMode);
#endif

    sal_uInt32 nCommandArgs = rtl_getAppCommandArgCount();
    if (nCommandArgs < 1)
    {
        std::cerr << "Usage: cppunittester <shared-library-path>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string testlib;
    {
        rtl::OUString path;
        rtl_getAppCommandArg(0, &path.pData);
        testlib = rtl::OUStringToOString(path, osl_getThreadTextEncoding()).getStr();
    }
    std::string args = testlib;
    for (sal_uInt32 i = 1; i < nCommandArgs; ++i)
    {
        rtl::OUString arg;
        rtl_getAppCommandArg(i, &arg.pData);
        args += ' ';
        args += rtl::OUStringToOString(arg, osl_getThreadTextEncoding()).getStr();
    }
    CppUnit::PlugInManager manager;
    manager.load(testlib, args);
    CppUnit::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    CppUnit::TestResult result;
    CppUnit::TestResultCollector collector;
    result.addListener(&collector);
    runner.run(result);
    CppUnit::CompilerOutputter(&collector, CppUnit::stdCErr()).write();
    return collector.wasSuccessful() ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
