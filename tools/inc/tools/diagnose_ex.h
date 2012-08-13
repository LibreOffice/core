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
#ifndef TOOLS_DIAGNOSE_EX_H
#define TOOLS_DIAGNOSE_EX_H

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <boost/current_function.hpp>

#define OSL_UNUSED( expression ) \
    (void)(expression)

#if OSL_DEBUG_LEVEL > 0
    #include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
    #include <com/sun/star/task/ErrorCodeIOException.hpp>
    #include <cppuhelper/exc_hlp.hxx>
    #include <osl/diagnose.h>
    #include <osl/thread.h>
    #include <boost/current_function.hpp>
    #include <typeinfo>

    /** reports a caught UNO exception via OSL diagnostics

        Note that whenever you use this, it might be an indicator that your error
        handling is not correct ....
    */
    #define DBG_UNHANDLED_EXCEPTION()   \
        ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() ); \
        ::rtl::OString sMessage( "caught an exception!" ); \
        sMessage += "\nin function:"; \
        sMessage += BOOST_CURRENT_FUNCTION; \
        sMessage += "\ntype: "; \
        sMessage += ::rtl::OUStringToOString( caught.getValueTypeName(), osl_getThreadTextEncoding() ); \
        ::com::sun::star::uno::Exception exception; \
        caught >>= exception; \
        if ( !exception.Message.isEmpty() ) \
        { \
            sMessage += "\nmessage: "; \
            sMessage += ::rtl::OUStringToOString( exception.Message, osl_getThreadTextEncoding() ); \
        } \
        if ( exception.Context.is() ) \
        { \
            const char* pContext = typeid( *exception.Context.get() ).name(); \
            sMessage += "\ncontext: "; \
            sMessage += pContext; \
        } \
        { \
            ::com::sun::star::configuration::CorruptedConfigurationException \
                specialized; \
            if ( caught >>= specialized ) \
            { \
                sMessage += "\ndetails: "; \
                sMessage += ::rtl::OUStringToOString( \
                    specialized.Details, osl_getThreadTextEncoding() ); \
            } \
        } \
        { \
            ::com::sun::star::task::ErrorCodeIOException specialized; \
            if ( caught >>= specialized ) \
            { \
                sMessage += "\ndetails: "; \
                sMessage += ::rtl::OString::valueOf( specialized.ErrCode ); \
            } \
        } \
        sMessage += "\n"; \
        OSL_ENSURE( false, sMessage.getStr() )

#else   // OSL_DEBUG_LEVEL
    #define DBG_UNHANDLED_EXCEPTION()
#endif  // OSL_DEBUG_LEVEL

/** This macro asserts the given condition (in debug mode), and throws
    an IllegalArgumentException afterwards.
 */
#define ENSURE_ARG_OR_THROW(c, m) if( !(c) ) { \
                                     OSL_ENSURE(c, m); \
                                     throw ::com::sun::star::lang::IllegalArgumentException( \
                                     ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(BOOST_CURRENT_FUNCTION)) + \
                                     ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ",\n" m )), \
                                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >(), \
                                     0 ); }
#define ENSURE_ARG_OR_THROW2(c, m, ifc, arg) if( !(c) ) { \
                                               OSL_ENSURE(c, m); \
                                               throw ::com::sun::star::lang::IllegalArgumentException( \
                                               ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(BOOST_CURRENT_FUNCTION)) + \
                                               ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ",\n" m )), \
                                               ifc, \
                                               arg ); }

/** This macro asserts the given condition (in debug mode), and throws
    an RuntimeException afterwards.
 */
#define ENSURE_OR_THROW(c, m) \
    if( !(c) ){ \
        OSL_ENSURE(c, m); \
        throw ::com::sun::star::uno::RuntimeException( \
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(BOOST_CURRENT_FUNCTION)) + \
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ",\n" m )), \
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() ); }

#define ENSURE_OR_THROW2(c, m, ifc) \
    if( !(c) ) { \
        OSL_ENSURE(c, m); \
        throw ::com::sun::star::uno::RuntimeException( \
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(BOOST_CURRENT_FUNCTION)) + \
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ",\n" m )), \
        ifc ); }

/** This macro asserts the given condition (in debug mode), and
    returns the given value afterwards.
 */
#define ENSURE_OR_RETURN(c, m, r) if( !(c) ) { \
                                     OSL_ENSURE(c, m); \
                                     return r; }

/** This macro asserts the given condition (in debug mode), and
    returns false afterwards.
 */
#define ENSURE_OR_RETURN_FALSE(c, m) \
    ENSURE_OR_RETURN(c, m, false)

/** This macro asserts the given condition (in debug mode), and
    returns afterwards, without return value "void".
 */
#define ENSURE_OR_RETURN_VOID( c, m ) \
    if( !(c) )  \
    { \
        OSL_ENSURE( c, m ); \
        return;   \
    }



/** This macro asserts the given condition (in debug mode), and
    returns afterwards, without return value "void".
 */
#define ENSURE_OR_RETURN_VOID( c, m ) \
    if( !(c) )  \
    { \
        OSL_ENSURE( c, m ); \
        return;   \
    }

/** asserts a given condition (in debug mode), and continues the most-inner
    loop if the condition is not met
*/
#define ENSURE_OR_CONTINUE( c, m ) \
    if ( !(c) ) \
    {   \
        OSL_ENSURE( false, m ); \
        continue;   \
    }

/** asserts a given condition (in debug mode), and continues the most-inner
    loop if the condition is not met
*/
#define ENSURE_OR_BREAK( c, m ) \
    if ( !(c) ) \
    {   \
        OSL_ENSURE( false, m ); \
        break;  \
    }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
