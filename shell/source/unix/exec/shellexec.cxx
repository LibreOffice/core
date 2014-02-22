/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_folders.h>

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>

#include <rtl/uri.hxx>
#include "shellexec.hxx"
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>

#include "uno/current_context.hxx"

#include <string.h>
#include <errno.h>
#include <unistd.h>





using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using osl::FileBase;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

#define SHELLEXEC_IMPL_NAME  "com.sun.star.comp.system.SystemShellExecute2"





namespace 
{
    Sequence< OUString > SAL_CALL ShellExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.sys.shell.SystemShellExecute";
        return aRet;
    }
}

void escapeForShell( OStringBuffer & rBuffer, const OString & rURL)
{
    sal_Int32 nmax = rURL.getLength();
    for(sal_Int32 n=0; n < nmax; ++n)
    {
        
        sal_Char c = rURL[n];
        if( ( c < 'A' || c > 'Z' ) && ( c < 'a' || c > 'z' ) && ( c < '0' || c > '9' )  && c != '/' && c != '.' )
            rBuffer.append( '\\' );

        rBuffer.append( c );
    }
}



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
    } catch (const RuntimeException &) {
    }
}



void SAL_CALL ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
    throw (IllegalArgumentException, SystemShellExecuteException, RuntimeException)
{
    OStringBuffer aBuffer, aLaunchBuffer;

    
    static const char *pDesktopLaunch = getenv( "DESKTOP_LAUNCH" );

    
    css::uno::Reference< css::uri::XUriReference > uri(
        css::uri::UriReferenceFactory::create(m_xContext)->parse(aCommand));
    if (uri.is() && uri->isAbsolute())
    {
        
        
        
        
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
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        aBuffer.append("open --");
#else
        
        
        com::sun::star::uno::Reference< com::sun::star::util::XMacroExpander >
            exp = com::sun::star::util::theMacroExpander::get(m_xContext);
        OUString aProgramURL;
        try {
            aProgramURL = exp->expandMacros(
                OUString( "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/"));
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

        
        
        
        if ( !m_aDesktopEnvironment.isEmpty() )
        {
            OString aDesktopEnvironment(m_aDesktopEnvironment.toAsciiLowerCase());
            OStringBuffer aCopy(aTmp);

            aCopy.append(aDesktopEnvironment + "-open-url");

            if ( 0 == access( aCopy.getStr(), X_OK) )
            {
                aBuffer.append(aDesktopEnvironment + "-");
            }
        }

        aBuffer.append("open-url");
#endif
        aBuffer.append(" ");
        escapeForShell(aBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));

        if ( pDesktopLaunch && *pDesktopLaunch )
        {
            aLaunchBuffer.append( OString(pDesktopLaunch) + " ");
            escapeForShell(aLaunchBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));
        }
    } else if ((nFlags & css::system::SystemShellExecuteFlags::URIS_ONLY) != 0)
    {
        throw css::lang::IllegalArgumentException(
            (OUString(
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

    
    if ( !aLaunchBuffer.isEmpty() )
    {
        FILE *pLaunch = popen( aLaunchBuffer.makeStringAndClear().getStr(), "w" );
        if ( pLaunch != NULL )
        {
            if ( 0 == pclose( pLaunch ) )
                return;
        }
        
        pDesktopLaunch = NULL;
    }

    OString cmd =
#ifdef LINUX
        
        "( " + aBuffer.makeStringAndClear() +  " ) &";
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


OUString SAL_CALL ShellExec::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString(SHELLEXEC_IMPL_NAME );
}


sal_Bool SAL_CALL ShellExec::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}


Sequence< OUString > SAL_CALL ShellExec::getSupportedServiceNames(   )
    throw( RuntimeException )
{
    return ShellExec_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
