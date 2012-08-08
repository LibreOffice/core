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

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>

#include <rtl/uri.hxx>
#include "shellexec.hxx"
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>

#include "uno/current_context.hxx"

#include <string.h>
#include <errno.h>
#include <unistd.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using rtl::OString;
using rtl::OUString;
using rtl::OStringBuffer;
using rtl::OUStringBuffer;
using osl::FileBase;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

#define SHELLEXEC_IMPL_NAME  "com.sun.star.comp.system.SystemShellExecute2"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{
    namespace css = com::sun::star;

    Sequence< OUString > SAL_CALL ShellExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString("com.sun.star.sys.shell.SystemShellExecute");
        return aRet;
    }
}

void escapeForShell( rtl::OStringBuffer & rBuffer, const rtl::OString & rURL)
{
    sal_Int32 nmax = rURL.getLength();
    for(sal_Int32 n=0; n < nmax; ++n)
    {
        // escape every non alpha numeric characters (excluding a few "known good") by prepending a '\'
        sal_Char c = rURL[n];
        if( ( c < 'A' || c > 'Z' ) && ( c < 'a' || c > 'z' ) && ( c < '0' || c > '9' )  && c != '/' && c != '.' )
            rBuffer.append( '\\' );

        rBuffer.append( c );
    }
}

//-----------------------------------------------------------------------------------------

ShellExec::ShellExec( const Reference< XComponentContext >& xContext ) :
    WeakImplHelper2< XSystemShellExecute, XServiceInfo >(),
    m_xContext(xContext)
{
    try {
        Reference< XCurrentContext > xCurrentContext(getCurrentContext());

        if (xCurrentContext.is())
        {
            Any aValue = xCurrentContext->getValueByName(
                OUString( "system.desktop-environment"  ) );

            OUString aDesktopEnvironment;
            if (aValue >>= aDesktopEnvironment)
            {
                m_aDesktopEnvironment = OUStringToOString(aDesktopEnvironment, RTL_TEXTENCODING_ASCII_US);
            }
        }
    } catch (const RuntimeException &e) {
    }
}

//-------------------------------------------------

void SAL_CALL ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
    throw (IllegalArgumentException, SystemShellExecuteException, RuntimeException)
{
    OStringBuffer aBuffer, aLaunchBuffer;

    // DESKTOP_LAUNCH, see http://freedesktop.org/pipermail/xdg/2004-August/004489.html
    static const char *pDesktopLaunch = getenv( "DESKTOP_LAUNCH" );

    // Check whether aCommand contains an absolute URI reference:
    css::uno::Reference< css::uri::XUriReference > uri(
        css::uri::UriReferenceFactory::create(m_xContext)->parse(aCommand));
    if (uri.is() && uri->isAbsolute())
    {
        // It seems to be a url ..
        // We need to re-encode file urls because osl_getFileURLFromSystemPath converts
        // to UTF-8 before encoding non ascii characters, which is not what other apps
        // expect.
        OUString aURL(
            com::sun::star::uri::ExternalUriReferenceTranslator::create(
                m_xContext)->translateToExternal(aCommand));
        if ( aURL.isEmpty() && !aCommand.isEmpty() )
        {
            throw RuntimeException(
                (OUString( "Cannot translate URI reference to external format: ")
                 + aCommand),
                static_cast< cppu::OWeakObject * >(this));
        }

#ifdef MACOSX
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
        aBuffer.append("open --");
#else
        // The url launchers are expected to be in the $BRAND_BASE_DIR/program
        // directory:
        com::sun::star::uno::Reference< com::sun::star::util::XMacroExpander >
            exp;
        if (!(m_xContext->getValueByName(
                  rtl::OUString( "/singletons/com.sun.star.util.theMacroExpander"))
              >>= exp)
            || !exp.is())
        {
            throw SystemShellExecuteException(
                rtl::OUString(
                        "component context fails to supply singleton"
                        " com.sun.star.util.theMacroExpander of type"
                        " com.sun.star.util.XMacroExpander"),
                static_cast< XSystemShellExecute * >(this), ENOENT);
        }
        OUString aProgramURL;
        try {
            aProgramURL = exp->expandMacros(
                rtl::OUString( "$BRAND_BASE_DIR/program/"));
        } catch (com::sun::star::lang::IllegalArgumentException &)
        {
            throw SystemShellExecuteException(
                OUString("Could not expand $BRAND_BASE_DIR path"),
                static_cast < XSystemShellExecute * > (this), ENOENT );
        }

        OUString aProgram;
        if ( FileBase::E_None != FileBase::getSystemPathFromFileURL(aProgramURL, aProgram))
        {
            throw SystemShellExecuteException(
                OUString("Cound not convert executable path"),
                static_cast < XSystemShellExecute * > (this), ENOENT );
        }

        OString aTmp = OUStringToOString(aProgram, osl_getThreadTextEncoding());
        escapeForShell(aBuffer, aTmp);

#ifdef SOLARIS
        if ( m_aDesktopEnvironment.getLength() == 0 )
             m_aDesktopEnvironment = OString("GNOME");
#endif

        // Respect the desktop environment - if there is an executable named
        // <desktop-environement-is>-open-url, pass the url to this one instead
        // of the default "open-url" script.
        if ( !m_aDesktopEnvironment.isEmpty() )
        {
            OString aDesktopEnvironment(m_aDesktopEnvironment.toAsciiLowerCase());
            OStringBuffer aCopy(aTmp);

            aCopy.append(aDesktopEnvironment);
            aCopy.append("-open-url");

            if ( 0 == access( aCopy.getStr(), X_OK) )
            {
                aBuffer.append(aDesktopEnvironment);
                aBuffer.append("-");
            }
        }

        aBuffer.append("open-url");
#endif
        aBuffer.append(" ");
        escapeForShell(aBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));

        if ( pDesktopLaunch && *pDesktopLaunch )
        {
            aLaunchBuffer.append( pDesktopLaunch );
            aLaunchBuffer.append(" ");
            escapeForShell(aLaunchBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));
        }
    } else if ((nFlags & css::system::SystemShellExecuteFlags::URIS_ONLY) != 0)
    {
        throw css::lang::IllegalArgumentException(
            (rtl::OUString(
                    "XSystemShellExecute.execute URIS_ONLY with non-absolute"
                    " URI reference ")
             + aCommand),
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
    if ( aLaunchBuffer.getLength() > 0 )
    {
        FILE *pLaunch = popen( aLaunchBuffer.makeStringAndClear().getStr(), "w" );
        if ( pLaunch != NULL )
        {
            if ( 0 == pclose( pLaunch ) )
                return;
        }
        // Failed, do not try DESKTOP_LAUNCH any more
        pDesktopLaunch = NULL;
    }

    OString cmd =
#ifdef LINUX
        // avoid blocking (call it in background)
        OStringBuffer().append( "( " ).append( aBuffer.makeStringAndClear() ).append( " ) &" ).makeStringAndClear();
#else
        aBuffer.makeStringAndClear();
#endif
    if ( 0 != pclose(popen(cmd.getStr(), "w")) )
    {
        int nerr = errno;
        throw SystemShellExecuteException(OUString::createFromAscii( strerror( nerr ) ),
            static_cast < XSystemShellExecute * > (this), nerr );
    }
}


// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL ShellExec::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString(SHELLEXEC_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL ShellExec::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = ShellExec_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL ShellExec::getSupportedServiceNames(   )
    throw( RuntimeException )
{
    return ShellExec_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
