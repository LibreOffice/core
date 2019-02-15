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
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#include <com/sun/star/deployment/DependencyException.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <com/sun/star/document/UndoFailedException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ldap/LdapGenericException.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/provider/ScriptExceptionRaisedException.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <comphelper/anytostring.hxx>
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

OString exceptionToString(const css::uno::Any & caught)
{
    auto toOString = [](OUString const & s) {
        return OUStringToOString( s, osl_getThreadTextEncoding() );
    };
    OString sMessage = toOString(caught.getValueTypeName());
    css::uno::Exception exception;
    caught >>= exception;
    if ( !exception.Message.isEmpty() )
    {
        sMessage += " message: ";
        sMessage += toOString(exception.Message);
    }
    if ( exception.Context.is() )
    {
        const char* pContext = typeid( *exception.Context.get() ).name();
#if defined __GLIBCXX__
        // demangle the type name, not necessary under windows, we already get demangled names there
        int status;
        pContext = abi::__cxa_demangle( pContext, nullptr, nullptr, &status);
#endif
        sMessage += " context: ";
        sMessage += pContext;
    }
    {
        css::configuration::CorruptedConfigurationException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " details: ";
            sMessage += toOString(specialized.Details);
        }
    }
    {
        css::configuration::InvalidBootstrapFileException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " BootstrapFileURL: ";
            sMessage += toOString(specialized.BootstrapFileURL);
        }
    }
    {
        css::configuration::MissingBootstrapFileException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " BootstrapFileURL: ";
            sMessage += toOString(specialized.BootstrapFileURL);
        }
    }
    {
        css::configuration::backend::MalformedDataException specialized;
        if ( caught >>= specialized )
        {
            sMessage += "\n    wrapped: ";
            sMessage += exceptionToString(specialized.ErrorDetails);
        }
    }
    {
        css::configuration::backend::BackendSetupException specialized;
        if ( caught >>= specialized )
        {
            sMessage += "\n    wrapped: ";
            sMessage += exceptionToString(specialized.BackendException);
        }
    }
    {
        css::deployment::DependencyException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " UnsatisfiedDependencies: ";
            sMessage += toOString(comphelper::anyToString(css::uno::Any(specialized.UnsatisfiedDependencies)));
        }
    }
    {
        css::deployment::DeploymentException specialized;
        if ( caught >>= specialized )
        {
            sMessage += "\n    wrapped: ";
            sMessage += exceptionToString(specialized.Cause);
        }
    }
    {
        css::document::CorruptedFilterConfigurationException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " Details: ";
            sMessage += toOString(specialized.Details);
        }
    }
    {
        css::document::UndoFailedException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " Reason: ";
            sMessage += toOString(comphelper::anyToString(specialized.Reason));
        }
    }
    {
        css::lang::IllegalArgumentException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " ArgumentPosition: ";
            sMessage += OString::number(specialized.ArgumentPosition);
        }
    }
    {
        css::lang::WrappedTargetException specialized;
        if ( caught >>= specialized )
        {
            sMessage += "\n    wrapped: ";
            sMessage += exceptionToString(specialized.TargetException);
        }
    }
    {
        css::lang::WrappedTargetRuntimeException specialized;
        if ( caught >>= specialized )
        {
            sMessage += "\n    wrapped: ";
            sMessage += exceptionToString(specialized.TargetException);
        }
    }
    {
        css::ldap::LdapGenericException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " ErrorCode: ";
            sMessage += OString::number(specialized.ErrorCode);
        }
    }
    {
        css::script::BasicErrorException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " ErrorCode: ";
            sMessage += OString::number(specialized.ErrorCode);
            sMessage += " ErrorMessageArgument: ";
            sMessage += toOString(specialized.ErrorMessageArgument);
        }
    }
    {
        css::script::CannotConvertException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " DestinationTypeClass: ";
            sMessage += toOString(comphelper::anyToString(css::uno::Any(specialized.DestinationTypeClass)));
            sMessage += " Reason: ";
            sMessage += OString::number(specialized.Reason);
            sMessage += " ArgumentIndex: ";
            sMessage += OString::number(specialized.ArgumentIndex);
        }
    }
    {
        css::script::provider::ScriptErrorRaisedException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " scriptName: ";
            sMessage += toOString(specialized.scriptName);
            sMessage += " language: ";
            sMessage += toOString(specialized.language);
            sMessage += " lineNum: ";
            sMessage += OString::number(specialized.lineNum);
        }
    }
    {
        css::script::provider::ScriptExceptionRaisedException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " exceptionType: ";
            sMessage += toOString(specialized.exceptionType);
        }
    }
    {
        css::script::provider::ScriptFrameworkErrorException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " scriptName: ";
            sMessage += toOString(specialized.scriptName);
            sMessage += " language: ";
            sMessage += toOString(specialized.language);
            sMessage += " errorType: ";
            sMessage += OString::number(specialized.errorType);
        }
    }
    {
        css::sdbc::SQLException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " SQLState: ";
            sMessage += toOString(specialized.SQLState);
            sMessage += " ErrorCode: ";
            sMessage += OString::number(specialized.ErrorCode);
            sMessage += "\n    wrapped: ";
            sMessage += exceptionToString(specialized.NextException);
        }
    }
    {
        css::system::SystemShellExecuteException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " PosixError: ";
            sMessage += OString::number(specialized.PosixError);
        }
    }
    {
        css::task::ErrorCodeIOException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " errcode: ";
            sMessage += OString::number( specialized.ErrCode );
        }
    }
    {
        css::ucb::CommandFailedException specialized;
        if ( caught >>= specialized )
        {
            sMessage += "\n    Reason: ";
            sMessage += exceptionToString( specialized.Reason );
        }
    }
    {
        css::ucb::ContentCreationException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " eError: ";
            sMessage += toOString(comphelper::anyToString( css::uno::Any(specialized.eError) ));
        }
    }
    {
        css::ucb::MissingPropertiesException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " Properties: ";
            sMessage += toOString(comphelper::anyToString( css::uno::Any(specialized.Properties) ));
        }
    }
    {
        css::ucb::NameClashException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " Name: ";
            sMessage += toOString( specialized.Name );
        }
    }
    {
        css::util::MalformedNumberFormatException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " CheckPos: ";
            sMessage += OString::number( specialized.CheckPos );
        }
    }
    {
        css::xml::dom::DOMException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " Code: ";
            sMessage += toOString(comphelper::anyToString( css::uno::Any(specialized.Code) ));
        }
    }
    {
        css::xml::dom::DOMException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " Code: ";
            sMessage += toOString(comphelper::anyToString( css::uno::Any(specialized.Code) ));
        }
    }
    {
        css::xml::sax::SAXException specialized;
        if ( caught >>= specialized )
        {
            sMessage += "\n    wrapped: ";
            sMessage += exceptionToString( specialized.WrappedException );
        }
    }
    {
        css::xml::sax::SAXParseException specialized;
        if ( caught >>= specialized )
        {
            sMessage += " PublicId: ";
            sMessage += toOString( specialized.PublicId );
            sMessage += " SystemId: ";
            sMessage += toOString( specialized.SystemId );
            sMessage += " LineNumber: ";
            sMessage += OString::number( specialized.LineNumber );
            sMessage += " ColumnNumber: ";
            sMessage += OString::number( specialized.ColumnNumber );
        }
    }
    return sMessage;
}

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
        sMessage += " exception: ";
        sMessage += exceptionToString(caught);

        if (area == nullptr)
            area = "legacy.osl";

        SAL_DETAIL_LOG_FORMAT(
            SAL_DETAIL_ENABLE_LOG_WARN, SAL_DETAIL_LOG_LEVEL_WARN,
            area, fileAndLineNo, "%s", sMessage.getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
