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

#include <tools/toolsdllapi.h>
#include <cppuhelper/exc_hlp.hxx>

TOOLS_DLLPUBLIC void DbgUnhandledException(const css::uno::Any& caughtException,
        const char* currentFunction, const char* fileAndLineNo,
        const char* area, const char* explanatory = nullptr);

//getCaughtException throws exceptions in never-going-to-happen situations which
//floods coverity with warnings
inline css::uno::Any DbgGetCaughtException()
{
#if defined(__COVERITY__)
    try
    {
        return ::cppu::getCaughtException();
    }
    catch (...)
    {
        std::abort();
    }
#else
    return ::cppu::getCaughtException();
#endif
}

/** reports a caught UNO exception via OSL diagnostics

    Note that whenever you use this, it might be an indicator that your error
    handling is not correct ....
    This takes two optional parameters: area and explanatory
*/
#define DBG_UNHANDLED_EXCEPTION_0_ARGS() \
    DbgUnhandledException( DbgGetCaughtException(), OSL_THIS_FUNC, SAL_DETAIL_WHERE );
#define DBG_UNHANDLED_EXCEPTION_1_ARGS(area) \
    DbgUnhandledException( DbgGetCaughtException(), OSL_THIS_FUNC, SAL_DETAIL_WHERE, area );
#define DBG_UNHANDLED_EXCEPTION_2_ARGS(area, explanatory) \
    DbgUnhandledException( DbgGetCaughtException(), OSL_THIS_FUNC, SAL_DETAIL_WHERE, area, explanatory );

#define DBG_UNHANDLED_FUNC_CHOOSER(_f1, _f2, _f3, ...) _f3
#define DBG_UNHANDLED_FUNC_RECOMPOSER(argsWithParentheses) DBG_UNHANDLED_FUNC_CHOOSER argsWithParentheses
#define DBG_UNHANDLED_CHOOSE_FROM_ARG_COUNT(...) DBG_UNHANDLED_FUNC_RECOMPOSER((__VA_ARGS__, DBG_UNHANDLED_EXCEPTION_2_ARGS, DBG_UNHANDLED_EXCEPTION_1_ARGS, DBG_UNHANDLED_EXCEPTION_0_ARGS, ))
#define DBG_UNHANDLED_NO_ARG_EXPANDER() ,,DBG_UNHANDLED_EXCEPTION_0_ARGS
#define DBG_UNHANDLED_MACRO_CHOOSER(...) DBG_UNHANDLED_CHOOSE_FROM_ARG_COUNT(DBG_UNHANDLED_NO_ARG_EXPANDER __VA_ARGS__ ())
#define DBG_UNHANDLED_EXCEPTION(...) DBG_UNHANDLED_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)


/** This macro asserts the given condition (in debug mode), and throws
    an IllegalArgumentException afterwards.
 */
#define ENSURE_ARG_OR_THROW(c, m) if( !(c) ) { \
                                     OSL_ENSURE(c, m); \
                                     throw css::lang::IllegalArgumentException( \
                                     OUStringLiteral(OSL_THIS_FUNC) \
                                     + ",\n" m, \
                                     css::uno::Reference< css::uno::XInterface >(), \
                                     0 ); }
#define ENSURE_ARG_OR_THROW2(c, m, ifc, arg) if( !(c) ) { \
                                               OSL_ENSURE(c, m); \
                                               throw css::lang::IllegalArgumentException( \
                                               OUStringLiteral(OSL_THIS_FUNC) \
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
        OUStringLiteral(OSL_THIS_FUNC) + ",\n" m, \
        css::uno::Reference< css::uno::XInterface >() ); }

#define ENSURE_OR_THROW2(c, m, ifc) \
    if( !(c) ) { \
        OSL_ENSURE(c, m); \
        throw css::uno::RuntimeException( \
        OUStringLiteral(OSL_THIS_FUNC) + ",\n" m, \
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

/** Convert a caught exception to a string suitable for logging.
*/
TOOLS_DLLPUBLIC OString exceptionToString(css::uno::Any const & caughtEx);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
