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
#ifndef INCLUDED_TOOLS_DIAGNOSE_EX_H
#define INCLUDED_TOOLS_DIAGNOSE_EX_H

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <boost/current_function.hpp>

#include <tools/toolsdllapi.h>

TOOLS_DLLPUBLIC void DbgUnhandledException(const css::uno::Any& caughtException, const char* currentFunction, const char* fileAndLineNo);

#if OSL_DEBUG_LEVEL > 0
    #include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
    #include <cppuhelper/exc_hlp.hxx>
    #include <osl/diagnose.h>
    #include <osl/thread.h>
    #include <boost/current_function.hpp>

    /** reports a caught UNO exception via OSL diagnostics

        Note that whenever you use this, it might be an indicator that your error
        handling is not correct ....
    */
    #define DBG_UNHANDLED_EXCEPTION()   \
        DbgUnhandledException( ::cppu::getCaughtException(), BOOST_CURRENT_FUNCTION, SAL_DETAIL_WHERE);

#else   // OSL_DEBUG_LEVEL
    #define DBG_UNHANDLED_EXCEPTION()
#endif  // OSL_DEBUG_LEVEL

/** This macro asserts the given condition (in debug mode), and throws
    an IllegalArgumentException afterwards.
 */
#define ENSURE_ARG_OR_THROW(c, m) if( !(c) ) { \
                                     OSL_ENSURE(c, m); \
                                     throw css::lang::IllegalArgumentException( \
                                     OUStringLiteral(BOOST_CURRENT_FUNCTION) \
                                     + ",\n" m, \
                                     css::uno::Reference< css::uno::XInterface >(), \
                                     0 ); }
#define ENSURE_ARG_OR_THROW2(c, m, ifc, arg) if( !(c) ) { \
                                               OSL_ENSURE(c, m); \
                                               throw css::lang::IllegalArgumentException( \
                                               OUStringLiteral(BOOST_CURRENT_FUNCTION) \
                                               + ",\n" m, \
                                               ifc, \
                                               arg ); }

/** This macro asserts the given condition (in debug mode), and throws
    an RuntimeException afterwards.
 */
#define ENSURE_OR_THROW(c, m) \
    if( !(c) ){ \
        OSL_ENSURE(c, m); \
        throw css::uno::RuntimeException( \
        OUStringLiteral(BOOST_CURRENT_FUNCTION) + ",\n" m, \
        css::uno::Reference< css::uno::XInterface >() ); }

#define ENSURE_OR_THROW2(c, m, ifc) \
    if( !(c) ) { \
        OSL_ENSURE(c, m); \
        throw css::uno::RuntimeException( \
        OUStringLiteral(BOOST_CURRENT_FUNCTION) + ",\n" m, \
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
