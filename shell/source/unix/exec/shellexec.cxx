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

#include <osl/thread.h>
#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include "shellexec.hxx"
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/lok.hxx>

#include <string.h>
#include <errno.h>

#if defined MACOSX
#include <sys/stat.h>
#endif

using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

namespace
{
    void escapeForShell( OStringBuffer & rBuffer, const OString & rURL)
    {
        sal_Int32 nmax = rURL.getLength();
        for(sal_Int32 n=0; n < nmax; ++n)
        {
            // escape every non alpha numeric characters (excluding a few "known good") by prepending a '\'
            char c = rURL[n];
            if( ( c < 'A' || c > 'Z' ) && ( c < 'a' || c > 'z' ) && ( c < '0' || c > '9' )  && c != '/' && c != '.' )
                rBuffer.append( '\\' );

            rBuffer.append( c );
        }
    }
}

ShellExec::ShellExec( const Reference< XComponentContext >& xContext ) :
    WeakImplHelper< XSystemShellExecute, XServiceInfo >(),
    m_xContext(xContext)
{
}

void SAL_CALL ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
{
    OStringBuffer aBuffer, aLaunchBuffer;

    if (comphelper::LibreOfficeKit::isActive())
    {
        SAL_WARN("shell", "Unusual - shell attempt to launch " << aCommand << " with params " << aParameter << " under lok");
        return;
    }

    // DESKTOP_LAUNCH, see http://freedesktop.org/pipermail/xdg/2004-August/004489.html
    static const char *pDesktopLaunch = getenv( "DESKTOP_LAUNCH" );

    // Check whether aCommand contains an absolute URI reference:
    css::uno::Reference< css::uri::XUriReference > uri(
        css::uri::UriReferenceFactory::create(m_xContext)->parse(aCommand));
    if (uri.is() && uri->isAbsolute())
    {
        // It seems to be a URL...
        // We need to re-encode file urls because osl_getFileURLFromSystemPath converts
        // to UTF-8 before encoding non ascii characters, which is not what other apps
        // expect.
        OUString aURL = css::uri::ExternalUriReferenceTranslator::create(
                            m_xContext)->translateToExternal(aCommand);
        if ( aURL.isEmpty() && !aCommand.isEmpty() )
        {
            throw RuntimeException(
                "Cannot translate URI reference to external format: "
                 + aCommand,
                static_cast< cppu::OWeakObject * >(this));
        }

#ifdef MACOSX
        bool dir = false;
        if (uri->getScheme().equalsIgnoreAsciiCase("file")) {
            OUString pathname;
            auto const e1 = osl::FileBase::getSystemPathFromFileURL(aCommand, pathname);
            if (e1 != osl::FileBase::E_None) {
                throw css::lang::IllegalArgumentException(
                    ("XSystemShellExecute.execute, getSystemPathFromFileURL <" + aCommand
                     + "> failed with " + OUString::number(e1)),
                    {}, 0);
            }
            OString pathname8;
            if (!pathname.convertToString(
                    &pathname8, RTL_TEXTENCODING_UTF8,
                    (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                     | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
            {
                throw css::lang::IllegalArgumentException(
                    "XSystemShellExecute.execute, cannot convert \"" + pathname + "\" to UTF-8", {},
                    0);
            }
            struct stat st;
            auto const e2 = stat(pathname8.getStr(), &st);
            if (e2 != 0) {
                auto const e3 = errno;
                SAL_INFO("shell", "stat(" << pathname8 << ") failed with errno " << e3);
            }
            if (e2 == 0 && S_ISDIR(st.st_mode)) {
                dir = true;
            } else if (e2 != 0 || !S_ISREG(st.st_mode)
                       || (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0)
            {
                throw css::lang::IllegalArgumentException(
                    "XSystemShellExecute.execute, cannot process <" + aCommand + ">", {}, 0);
            } else if (pathname.endsWithIgnoreAsciiCase(".class")
                       || pathname.endsWithIgnoreAsciiCase(".fileloc")
                       || pathname.endsWithIgnoreAsciiCase(".jar"))
            {
                dir = true;
            }
        }

        //TODO: Using open(1) with an argument that syntactically is an absolute
        // URI reference does not necessarily give expected results:
        // 1  If the given URI reference matches a supported scheme (e.g.,
        //  "mailto:foo"):
        // 1.1  If it matches an existing pathname (relative to CWD):  Results
        //  in "mailto:foo?\n[0]\tcancel\n[1]\tOpen the file\tmailto:foo\n[2]\t
        //  Open the URL\tmailto:foo\n\nWhich did you mean? Cancelled." on
        //  stderr and SystemShellExecuteException.
        // 1.2  If it does not match an exitsting pathname (relative to CWD):
        //  Results in the corresponding application being opened with the given
        //  document (e.g., Mail with a New Message).
        // 2  If the given URI reference does not match a supported scheme
        //  (e.g., "foo:bar"):
        // 2.1  If it matches an existing pathname (relative to CWD) pointing to
        //  an executable:  Results in execution of that executable.
        // 2.2  If it matches an existing pathname (relative to CWD) pointing to
        //  a non-executable regular file:  Results in opening it in TextEdit.
        // 2.3  If it matches an existing pathname (relative to CWD) pointing to
        //  a directory:  Results in opening it in Finder.
        // 2.4  If it does not match an exitsting pathname (relative to CWD):
        //  Results in "The file /.../foo:bar does not exits." (where "/..." is
        //  the CWD) on stderr and SystemShellExecuteException.
        aBuffer.append("open");
        if (dir) {
            aBuffer.append(" -R");
        }
        aBuffer.append(" --");
#else
        // Just use xdg-open on non-Mac
        aBuffer.append("/usr/bin/xdg-open");
#endif
        aBuffer.append(" ");
        escapeForShell(aBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));

        if ( pDesktopLaunch && *pDesktopLaunch )
        {
            aLaunchBuffer.append( pDesktopLaunch + OString::Concat(" "));
            escapeForShell(aLaunchBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));
        }
    } else if ((nFlags & css::system::SystemShellExecuteFlags::URIS_ONLY) != 0)
    {
        throw css::lang::IllegalArgumentException(
            "XSystemShellExecute.execute URIS_ONLY with non-absolute"
                     " URI reference "
             + aCommand,
            static_cast< cppu::OWeakObject * >(this), 0);
    } else {
        escapeForShell(aBuffer, OUStringToOString(aCommand, osl_getThreadTextEncoding()));
        aBuffer.append(" ");
        if( nFlags != 42 )
            escapeForShell(aBuffer, OUStringToOString(aParameter, osl_getThreadTextEncoding()));
        else
            aBuffer.append(OUStringToOString(aParameter, osl_getThreadTextEncoding()));
    }

    // Prefer DESKTOP_LAUNCH when available
    if ( !aLaunchBuffer.isEmpty() )
    {
        FILE *pLaunch = popen( aLaunchBuffer.makeStringAndClear().getStr(), "w" );
        if ( pLaunch != nullptr )
        {
            if ( 0 == pclose( pLaunch ) )
                return;
        }
        // Failed, do not try DESKTOP_LAUNCH any more
        pDesktopLaunch = nullptr;
    }

    OString cmd =
#ifdef LINUX
        // avoid blocking (call it in background)
        "( " + aBuffer.makeStringAndClear() +  " ) &";
#else
        aBuffer.makeStringAndClear();
#endif
    FILE *pLaunch = popen(cmd.getStr(), "w");
    if ( pLaunch != nullptr )
    {
        if ( 0 == pclose( pLaunch ) )
            return;
    }

    int nerr = errno;
    throw SystemShellExecuteException(OUString::createFromAscii( strerror( nerr ) ),
        static_cast < XSystemShellExecute * > (this), nerr );
}

// XServiceInfo

OUString SAL_CALL ShellExec::getImplementationName(  )
{
    return "com.sun.star.comp.system.SystemShellExecute";
}

sal_Bool SAL_CALL ShellExec::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL ShellExec::getSupportedServiceNames(   )
{
    return { "com.sun.star.system.SystemShellExecute" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
shell_ShellExec_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ShellExec(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
