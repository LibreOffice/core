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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_H
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_H

#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/deployment/XPackageRegistry.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include "dp_misc_api.hxx"

#define ARLEN(x) (sizeof (x) / sizeof *(x))

namespace dp_misc {

const sal_Char CR = 0x0d;
const sal_Char LF = 0x0a;


class MutexHolder
{
    mutable ::osl::Mutex m_mutex;
protected:
    inline ::osl::Mutex & getMutex() const { return m_mutex; }
};


inline void try_dispose( css::uno::Reference< css::uno::XInterface> const & x )
{
    css::uno::Reference< css::lang::XComponent> xComp( x, css::uno::UNO_QUERY );
    if (xComp.is())
        xComp->dispose();
}




DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
OUString expandUnoRcTerm( OUString const & term );

DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
OUString makeRcTerm( OUString const & url );


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
OUString expandUnoRcUrl( OUString const & url );



/** appends a relative path to a url.

    The relative path must already be correctly encoded for use in an URL.
    If the URL starts with vnd.sun.star.expand then the relative path will
    be again encoded for use in an "expand" URL.
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString makeURL(
    OUString const & baseURL, OUString const & relPath );


/** appends a relative path to a url.

    This is the same as makeURL, but the relative Path must me a segment
    of an system path.
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString makeURLAppendSysPathSegment(
    OUString const & baseURL, OUString const & relPath );


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC OUString generateRandomPipeId();

class AbortChannel;

DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
css::uno::Reference< css::uno::XInterface> resolveUnoURL(
    OUString const & connectString,
    css::uno::Reference< css::uno::XComponentContext> const & xLocalContext,
    AbortChannel * abortChannel = NULL );


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool office_is_running();


DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
oslProcess raiseProcess( OUString const & appURL,
                         css::uno::Sequence< OUString > const & args );



/** writes the argument string to the console.
    On Linux/Unix/etc. it converts the UTF16 string to an ANSI string using
    osl_getThreadTextEncoding() as target encoding. On Windows it uses WriteFile
    with the standard out stream. unopkg.com reads the data and prints them out using
    WriteConsoleW.
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
void writeConsole(OUString const & sText);

/** writes the argument to the console using the error stream.
    Otherwise the same as writeConsole.
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
void writeConsoleError(OUString const & sText);


/** reads from the console.
    On Linux/Unix/etc. it uses fgets to read char values and converts them to OUString
    using osl_getThreadTextEncoding as target encoding. The returned string has a maximum
    size of 1024 and does NOT include leading and trailing white space(applied OUString::trim())
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
OUString readConsole();

/** print the text to the console in a debug build.
    The argument is forwarded to writeConsole. The function does not add new line.
    The code is only executed if  OSL_DEBUG_LEVEL > 1
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
void TRACE(OUString const & sText);

/** registers or revokes shared or bundled extensions which have been
    recently added or removed.
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
void syncRepositories(
    bool force,
    css::uno::Reference<
        css::ucb::XCommandEnvironment> const & xCmdEnv);

/** workaround: for some reason the bridge threads which communicate with the
    uno.exe process are not released on time
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
void disposeBridges(
    css::uno::Reference< css::uno::XComponentContext >
        const & ctx);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
