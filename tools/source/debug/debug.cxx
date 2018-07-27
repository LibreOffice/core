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

#if defined (UNX) || defined (__GNUC__)
#include <unistd.h>
#endif

#include <errno.h>
#include <time.h>
#include <cstdarg>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <tools/debug.hxx>
#include <rtl/string.h>
#include <sal/log.hxx>
#include <sal/macros.h>
#include <osl/thread.h>

#include <typeinfo>
#include <vector>

#include <tools/diagnose_ex.h>

#if defined __GLIBCXX__
#include <cxxabi.h>
#endif

#ifndef NDEBUG

struct DebugData
{
    DbgTestSolarMutexProc   pDbgTestSolarMutex;
    bool                    bTestSolarMutexWasSet;

    DebugData()
        :pDbgTestSolarMutex( nullptr ), bTestSolarMutexWasSet(false)
    {
    }
};

static DebugData aDebugData;

void DbgSetTestSolarMutex( DbgTestSolarMutexProc pParam )
{
    aDebugData.pDbgTestSolarMutex = pParam;
    if (pParam)
        aDebugData.bTestSolarMutexWasSet = true;
}

void DbgTestSolarMutex()
{
    // don't warn if it was set at least once, because then we're probably just post-DeInitVCL()
    SAL_WARN_IF(
        !aDebugData.bTestSolarMutexWasSet && aDebugData.pDbgTestSolarMutex == nullptr, "tools.debug",
        "no DbgTestSolarMutex function set");
    if ( aDebugData.pDbgTestSolarMutex )
        aDebugData.pDbgTestSolarMutex();
}

#endif

void DbgUnhandledException(const css::uno::Any & caught, const char* currentFunction, const char* fileAndLineNo,
        const char* area, const char* explanatory)
{
        OString sMessage( "DBG_UNHANDLED_EXCEPTION in " );
        sMessage += currentFunction;
        if (explanatory)
        {
            sMessage += "\n    when: ";
            sMessage += explanatory;
        }
        sMessage += "\n    type: ";
        sMessage += OUStringToOString( caught.getValueTypeName(), osl_getThreadTextEncoding() );
        css::uno::Exception exception;
        caught >>= exception;
        if ( !exception.Message.isEmpty() )
        {
            sMessage += "\n    message: ";
            sMessage += OUStringToOString( exception.Message, osl_getThreadTextEncoding() );
        }
        if ( exception.Context.is() )
        {
            const char* pContext = typeid( *exception.Context.get() ).name();
#if defined __GLIBCXX__
            // demangle the type name, not necessary under windows, we already get demangled names there
            int status;
            pContext = abi::__cxa_demangle( pContext, nullptr, nullptr, &status);
#endif
            sMessage += "\n    context: ";
            sMessage += pContext;
        }
        {
            css::configuration::CorruptedConfigurationException
                specialized;
            if ( caught >>= specialized )
            {
                sMessage += "\n    details: ";
                sMessage += OUStringToOString(
                    specialized.Details, osl_getThreadTextEncoding() );
            }
        }
        {
            css::task::ErrorCodeIOException specialized;
            if ( caught >>= specialized )
            {
                sMessage += "\n    details: ";
                sMessage += OString::number( specialized.ErrCode );
            }
        }
        sMessage += "\n";

        if (area == nullptr)
            area = "legacy.osl";

        SAL_DETAIL_LOG_FORMAT(
            SAL_DETAIL_ENABLE_LOG_WARN, SAL_DETAIL_LOG_LEVEL_WARN,
            area, fileAndLineNo, "%s", sMessage.getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
