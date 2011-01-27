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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
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

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define SHELLEXEC_IMPL_NAME  "com.sun.star.comp.system.SystemShellExecute2"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{
    Sequence< OUString > SAL_CALL ShellExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sys.shell.SystemShellExecute"));
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
#ifndef OS2 // YD shell does not support escaped chars
        if( ( c < 'A' || c > 'Z' ) && ( c < 'a' || c > 'z' ) && ( c < '0' || c > '9' )  && c != '/' && c != '.' )
            rBuffer.append( '\\' );
#endif

        rBuffer.append( c );
    }
}

//-----------------------------------------------------------------------------------------
//
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
                OUString( RTL_CONSTASCII_USTRINGPARAM( "system.desktop-environment" ) ) );

            OUString aDesktopEnvironment;
            if (aValue >>= aDesktopEnvironment)
            {
                m_aDesktopEnvironment = OUStringToOString(aDesktopEnvironment, RTL_TEXTENCODING_ASCII_US);
            }
        }
    } catch (RuntimeException e) {
    }
}

//-------------------------------------------------
//
//-------------------------------------------------

void SAL_CALL ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
    throw (IllegalArgumentException, SystemShellExecuteException, RuntimeException)
{
    OStringBuffer aBuffer, aLaunchBuffer;

    // DESKTOP_LAUNCH, see http://freedesktop.org/pipermail/xdg/2004-August/004489.html
    static const char *pDesktopLaunch = getenv( "DESKTOP_LAUNCH" );

    // Check wether aCommand contains a document url or not
    sal_Int32 nIndex = aCommand.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM(":/") ) );

    if( nIndex > 0 || 0 == aCommand.compareToAscii("mailto:", 7) )
    {
        // It seems to be a url ..
        // We need to re-encode file urls because osl_getFileURLFromSystemPath converts
        // to UTF-8 before encoding non ascii characters, which is not what other apps
        // expect.
        OUString aURL(
            com::sun::star::uri::ExternalUriReferenceTranslator::create(
                m_xContext)->translateToExternal(aCommand));
        if ( aURL.getLength() == 0 && aCommand.getLength() != 0 )
        {
            throw RuntimeException(
                (OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot translate URI reference to external format: "))
                 + aCommand),
                static_cast< cppu::OWeakObject * >(this));
        }

#ifdef MACOSX
        aBuffer.append("open");
#else
        // The url launchers are expected to be in the $OOO_BASE_DIR/program
        // directory:
        com::sun::star::uno::Reference< com::sun::star::util::XMacroExpander >
            exp;
        if (!(m_xContext->getValueByName(
                  rtl::OUString(
                      RTL_CONSTASCII_USTRINGPARAM(
                          "/singletons/com.sun.star.util.theMacroExpander")))
              >>= exp)
            || !exp.is())
        {
            throw SystemShellExecuteException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "component context fails to supply singleton"
                        " com.sun.star.util.theMacroExpander of type"
                        " com.sun.star.util.XMacroExpander")),
                static_cast< XSystemShellExecute * >(this), ENOENT);
        }
        OUString aProgramURL;
        try {
            aProgramURL = exp->expandMacros(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/program/")));
        } catch (com::sun::star::lang::IllegalArgumentException &)
        {
            throw SystemShellExecuteException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Could not expand $OOO_BASE_DIR path")),
                static_cast < XSystemShellExecute * > (this), ENOENT );
        }

        OUString aProgram;
        if ( FileBase::E_None != FileBase::getSystemPathFromFileURL(aProgramURL, aProgram))
        {
            throw SystemShellExecuteException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Cound not convert executable path")),
                static_cast < XSystemShellExecute * > (this), ENOENT );
        }

#ifdef OS2
        OStringBuffer aProg = OUStringToOString(aProgram, osl_getThreadTextEncoding());
        aProg.append("open-url.exe");
        OString aUrl = OUStringToOString(aURL, osl_getThreadTextEncoding());
        if ( -1 == spawnl(P_NOWAIT, aProg.getStr(), aProg.getStr(), aUrl.getStr() , NULL) )
        {
            int nerr = errno;
            throw SystemShellExecuteException(OUString::createFromAscii( strerror( nerr ) ),
                static_cast < XSystemShellExecute * > (this), nerr );
        }
        return;
#endif

        OString aTmp = OUStringToOString(aProgram, osl_getThreadTextEncoding());
        escapeForShell(aBuffer, aTmp);

#ifdef SOLARIS
        if ( m_aDesktopEnvironment.getLength() == 0 )
             m_aDesktopEnvironment = OString("GNOME");
#endif

        // Respect the desktop environment - if there is an executable named
        // <desktop-environement-is>-open-url, pass the url to this one instead
        // of the default "open-url" script.
        if ( m_aDesktopEnvironment.getLength() > 0 )
        {
            OString aDesktopEnvironment(m_aDesktopEnvironment.toAsciiLowerCase());
            OStringBuffer aCopy(aTmp);

            aCopy.append(aDesktopEnvironment);
            aCopy.append("-open-url");

            if ( 0 == access( aCopy.getStr(), X_OK) )
            {
                aBuffer.append(aDesktopEnvironment);
                aBuffer.append("-");

                /* CDE requires file urls to be decoded */
                if ( m_aDesktopEnvironment.equals("CDE") && 0 == aURL.compareToAscii("file://", 7) )
                {
                    aURL = rtl::Uri::decode(aURL, rtl_UriDecodeWithCharset, osl_getThreadTextEncoding());
                }
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
        OStringBuffer().append( "( " ).append( aBuffer ).append( " ) &" ).makeStringAndClear();
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
    return OUString(RTL_CONSTASCII_USTRINGPARAM( SHELLEXEC_IMPL_NAME ));
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
