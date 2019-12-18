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

#include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#include <com/sun/star/deployment/DependencyException.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <com/sun/star/document/UndoFailedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
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
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/util/MalformedNumberFormatException.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <comphelper/anytostring.hxx>
#include <tools/debug.hxx>
#include <sal/log.hxx>
#include <osl/thread.h>
#include <rtl/strbuf.hxx>

#include <cstdlib>
#include <typeinfo>

#include <tools/diagnose_ex.h>

#if defined __GLIBCXX__
#include <cxxabi.h>
#endif

namespace {

struct DebugData
{
    DbgTestSolarMutexProc   pDbgTestSolarMutex;
    bool                    bTestSolarMutexWasSet;

    DebugData()
        :pDbgTestSolarMutex( nullptr ), bTestSolarMutexWasSet(false)
    {
    }
};

}

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

static void exceptionToStringImpl(OStringBuffer& sMessage, const css::uno::Any & caught)
{
    auto toOString = [](OUString const & s) {
        return OUStringToOString( s, osl_getThreadTextEncoding() );
    };
    sMessage.append(toOString(caught.getValueTypeName()));
    css::uno::Exception exception;
    caught >>= exception;
    if ( !exception.Message.isEmpty() )
    {
        sMessage.append(" message: ");
        sMessage.append(toOString(exception.Message));
    }
/*  TODO FIXME (see https://gerrit.libreoffice.org/#/c/83245/)
    if ( exception.Context.is() )
    {
        const char* pContext = typeid( *exception.Context ).name();
#if defined __GLIBCXX__
        // demangle the type name, not necessary under windows, we already get demangled names there
        int status;
        pContext = abi::__cxa_demangle( pContext, nullptr, nullptr, &status);
#endif
        sMessage.append(" context: ");
        sMessage.append(pContext);
#if defined __GLIBCXX__
        std::free(const_cast<char *>(pContext));
#endif
    }
*/
    {
        css::configuration::CorruptedConfigurationException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" details: ");
            sMessage.append(toOString(specialized.Details));
        }
    }
    {
        css::configuration::InvalidBootstrapFileException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" BootstrapFileURL: ");
            sMessage.append(toOString(specialized.BootstrapFileURL));
        }
    }
    {
        css::configuration::MissingBootstrapFileException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" BootstrapFileURL: ");
            sMessage.append(toOString(specialized.BootstrapFileURL));
        }
    }
    {
        css::configuration::backend::MalformedDataException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append("\n    wrapped: ");
            sMessage.append(exceptionToString(specialized.ErrorDetails));
        }
    }
    {
        css::configuration::backend::BackendSetupException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append("\n    wrapped: ");
            sMessage.append(exceptionToString(specialized.BackendException));
        }
    }
    {
        css::deployment::DependencyException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" UnsatisfiedDependencies: ");
            sMessage.append(toOString(comphelper::anyToString(css::uno::Any(specialized.UnsatisfiedDependencies))));
        }
    }
    {
        css::deployment::DeploymentException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append("\n    wrapped: ");
            sMessage.append(exceptionToString(specialized.Cause));
        }
    }
    {
        css::document::CorruptedFilterConfigurationException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" Details: ");
            sMessage.append(toOString(specialized.Details));
        }
    }
    {
        css::document::UndoFailedException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" Reason: ");
            sMessage.append(toOString(comphelper::anyToString(specialized.Reason)));
        }
    }
    {
        css::lang::IllegalArgumentException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" ArgumentPosition: ");
            sMessage.append(OString::number(specialized.ArgumentPosition));
        }
    }
    {
        css::lang::WrappedTargetException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append("\n    wrapped: ");
            sMessage.append(exceptionToString(specialized.TargetException));
        }
    }
    {
        css::lang::WrappedTargetRuntimeException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append("\n    wrapped: ");
            sMessage.append(exceptionToString(specialized.TargetException));
        }
    }
    {
        css::ldap::LdapGenericException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" ErrorCode: ");
            sMessage.append(OString::number(specialized.ErrorCode));
        }
    }
    {
        css::script::BasicErrorException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" ErrorCode: ");
            sMessage.append(OString::number(specialized.ErrorCode));
            sMessage.append(" ErrorMessageArgument: ");
            sMessage.append(toOString(specialized.ErrorMessageArgument));
        }
    }
    {
        css::script::CannotConvertException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" DestinationTypeClass: ");
            sMessage.append(toOString(comphelper::anyToString(css::uno::Any(specialized.DestinationTypeClass))));
            sMessage.append(" Reason: ");
            sMessage.append(OString::number(specialized.Reason));
            sMessage.append(" ArgumentIndex: ");
            sMessage.append(OString::number(specialized.ArgumentIndex));
        }
    }
    {
        css::script::provider::ScriptErrorRaisedException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" scriptName: ");
            sMessage.append(toOString(specialized.scriptName));
            sMessage.append(" language: ");
            sMessage.append(toOString(specialized.language));
            sMessage.append(" lineNum: ");
            sMessage.append(OString::number(specialized.lineNum));
        }
    }
    {
        css::script::provider::ScriptExceptionRaisedException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" exceptionType: ");
            sMessage.append(toOString(specialized.exceptionType));
        }
    }
    {
        css::script::provider::ScriptFrameworkErrorException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" scriptName: ");
            sMessage.append(toOString(specialized.scriptName));
            sMessage.append(" language: ");
            sMessage.append(toOString(specialized.language));
            sMessage.append(" errorType: ");
            sMessage.append(OString::number(specialized.errorType));
        }
    }
    {
        css::sdbc::SQLException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" SQLState: ");
            sMessage.append(toOString(specialized.SQLState));
            sMessage.append(" ErrorCode: ");
            sMessage.append(OString::number(specialized.ErrorCode));
            sMessage.append("\n    wrapped: ");
            sMessage.append(exceptionToString(specialized.NextException));
        }
    }
    {
        css::system::SystemShellExecuteException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" PosixError: ");
            sMessage.append(OString::number(specialized.PosixError));
        }
    }
    {
        css::task::ErrorCodeIOException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" errcode: ");
            sMessage.append(OString::number( specialized.ErrCode ));
        }
    }
    {
        css::ucb::CommandFailedException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append("\n    Reason: ");
            sMessage.append(exceptionToString( specialized.Reason ));
        }
    }
    {
        css::ucb::ContentCreationException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" eError: ");
            sMessage.append(toOString(comphelper::anyToString( css::uno::Any(specialized.eError) )));
        }
    }
    {
        css::ucb::MissingPropertiesException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" Properties: ");
            sMessage.append(toOString(comphelper::anyToString( css::uno::Any(specialized.Properties) )));
        }
    }
    {
        css::ucb::NameClashException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" Name: ");
            sMessage.append(toOString( specialized.Name ));
        }
    }
    {
        css::util::MalformedNumberFormatException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" CheckPos: ");
            sMessage.append(OString::number( specialized.CheckPos ));
        }
    }
    {
        css::xml::dom::DOMException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" Code: ");
            sMessage.append(toOString(comphelper::anyToString( css::uno::Any(specialized.Code) )));
        }
    }
    {
        css::xml::dom::DOMException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" Code: ");
            sMessage.append(toOString(comphelper::anyToString( css::uno::Any(specialized.Code) )));
        }
    }
    {
        css::xml::sax::SAXException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append("\n    wrapped: ");
            sMessage.append(exceptionToString( specialized.WrappedException ));
        }
    }
    {
        css::xml::sax::SAXParseException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" PublicId: ");
            sMessage.append(toOString( specialized.PublicId ));
            sMessage.append(" SystemId: ");
            sMessage.append(toOString( specialized.SystemId ));
            sMessage.append(" LineNumber: ");
            sMessage.append(OString::number( specialized.LineNumber ));
            sMessage.append(" ColumnNumber: ");
            sMessage.append(OString::number( specialized.ColumnNumber ));
        }
    }
    {
        css::ucb::InteractiveIOException specialized;
        if ( caught >>= specialized )
        {
            sMessage.append(" Code: ");
            sMessage.append(OString::number( static_cast<sal_Int32>(specialized.Code) ));
        }
    }
}

OString exceptionToString(const css::uno::Any & caught)
{
    OStringBuffer sMessage(512);
    exceptionToStringImpl(sMessage, caught);
    return sMessage.makeStringAndClear();
}

void DbgUnhandledException(const css::uno::Any & caught, const char* currentFunction, const char* fileAndLineNo,
        const char* area, const char* explanatory)
{
        OStringBuffer sMessage( 512 );
        sMessage.append( "DBG_UNHANDLED_EXCEPTION in " );
        sMessage.append(currentFunction);
        if (explanatory)
        {
            sMessage.append("\n    when: ");
            sMessage.append(explanatory);
        }
        sMessage.append(" exception: ");
        exceptionToStringImpl(sMessage, caught);

        if (area == nullptr)
            area = "legacy.osl";

        SAL_DETAIL_LOG_FORMAT(
            SAL_DETAIL_ENABLE_LOG_WARN, SAL_DETAIL_LOG_LEVEL_WARN,
            area, fileAndLineNo, "%s", sMessage.getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
