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

#include <config_folders.h>

#include <osl/diagnose.h>
#include <osl/thread.h>

#include <rtl/bootstrap.hxx>

#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include "cmdmailsuppl.hxx"
#include "cmdmailmsg.hxx"
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <string.h>
#include <errno.h>
#include <unistd.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::beans::PropertyValue;
using com::sun::star::system::XSimpleMailClientSupplier;
using com::sun::star::system::XSimpleMailClient;
using com::sun::star::system::XSimpleMailMessage;
using com::sun::star::system::XSimpleMailMessage2;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::NoSuchElementException;
using osl::MutexGuard;
using osl::FileBase;

using namespace cppu;
using namespace com::sun::star::system::SimpleMailClientFlags;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::configuration;

#define COMP_IMPL_NAME  "com.sun.star.comp.system.SimpleCommandMail2"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{
    Sequence< OUString > SAL_CALL Component_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.system.SimpleCommandMail";
        return aRet;
    }

} // end private namespace

//-------------------------------------------------

CmdMailSuppl::CmdMailSuppl( const Reference< XComponentContext >& xContext ) :
    WeakImplHelper3< XSimpleMailClientSupplier, XSimpleMailClient, XServiceInfo >()
{
    m_xConfigurationProvider = theDefaultProvider::get(xContext);
}

//-------------------------------------------------
// XSimpleMailClientSupplier
//-------------------------------------------------

Reference< XSimpleMailClient > SAL_CALL CmdMailSuppl::querySimpleMailClient(  )
    throw (RuntimeException)
{
    return static_cast < XSimpleMailClient * > (this);
}

//------------------------------------------------
// XSimpleMailClient
//------------------------------------------------

Reference< XSimpleMailMessage > SAL_CALL CmdMailSuppl::createSimpleMailMessage(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XSimpleMailMessage >( new CmdMailMsg(  ) );
}

//------------------------------------------------
// XSimpleMailClient
//------------------------------------------------

void SAL_CALL CmdMailSuppl::sendSimpleMailMessage( const Reference< XSimpleMailMessage >& xSimpleMailMessage, sal_Int32 /*aFlag*/ )
    throw (IllegalArgumentException, Exception, RuntimeException)
{
    if ( ! xSimpleMailMessage.is() )
    {
        throw ::com::sun::star::lang::IllegalArgumentException( "No message specified" ,
            static_cast < XSimpleMailClient * > (this), 1 );
    }

    if( ! m_xConfigurationProvider.is() )
    {
        throw ::com::sun::star::uno::Exception( "Can not access configuration" ,
            static_cast < XSimpleMailClient * > (this) );
    }


    OUString aProgramURL("$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/senddoc");
    rtl::Bootstrap::expandMacros(aProgramURL);

    OUString aProgram;
    if ( FileBase::E_None != FileBase::getSystemPathFromFileURL(aProgramURL, aProgram))
    {
        throw ::com::sun::star::uno::Exception("Cound not convert executable path",
            static_cast < XSimpleMailClient * > (this));
    }

    OStringBuffer aBuffer("\"" + OUStringToOString(aProgram, osl_getThreadTextEncoding()) + "\" ");

    try
    {
        // Query XNameAccess interface of the org.openoffice.Office.Common/ExternalMailer
        // configuration node to retriece the users preferred email application. This may
        // transparently by redirected to e.g. the corresponding GConf setting in GNOME.
        OUString aConfigRoot = "org.openoffice.Office.Common/ExternalMailer";

        PropertyValue aProperty;
        aProperty.Name = OUString("nodepath");
        aProperty.Value = makeAny( aConfigRoot );

        Sequence< Any > aArgumentList( 1 );
        aArgumentList[0] = makeAny( aProperty );

        Reference< XNameAccess > xNameAccess =
            Reference< XNameAccess > (
                m_xConfigurationProvider->createInstanceWithArguments(
                    OUString("com.sun.star.configuration.ConfigurationAccess"),
                    aArgumentList ),
                UNO_QUERY );

        if( xNameAccess.is() )
        {
            OUString aMailer;

            // Retrieve the value for "Program" node and append it feed senddoc with it
            // using the (undocumented) --mailclient switch
            xNameAccess->getByName("Program") >>= aMailer;

            if( !aMailer.isEmpty() )
            {
                // make sure we have a system path
                FileBase::getSystemPathFromFileURL( aMailer, aMailer );

                aBuffer.append("--mailclient " + OUStringToOString( aMailer, osl_getThreadTextEncoding() ) +
                               " ");
            }
#ifdef MACOSX
            else
                aBuffer.append("--mailclient Mail ");
#endif
        }

    }

    catch(const RuntimeException &e )
    {
        m_xConfigurationProvider.clear();
        OSL_TRACE( "RuntimeException caught accessing configuration provider." );
        OSL_TRACE( "%s", OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        throw;
    }

    Reference< XSimpleMailMessage2 > xMessage( xSimpleMailMessage, UNO_QUERY );
    if ( xMessage.is() )
    {
        rtl::OUString sBody = xMessage->getBody();
        if ( sBody.getLength() > 0 )
        {
            aBuffer.append("--body \"");
            aBuffer.append(OUStringToOString(sBody, osl_getThreadTextEncoding()));
            aBuffer.append("\" ");
        }
    }

    // Append originator if set in the message
    if ( !xSimpleMailMessage->getOriginator().isEmpty() )
    {
        aBuffer.append("--from \"" +
                        OUStringToOString(xSimpleMailMessage->getOriginator(), osl_getThreadTextEncoding()) +
                       "\" ");
    }

    // Append receipient if set in the message
    if ( !xSimpleMailMessage->getRecipient().isEmpty() )
    {
        aBuffer.append("--to \"" +
                       OUStringToOString(xSimpleMailMessage->getRecipient(), osl_getThreadTextEncoding()) +
                       "\" ");
    }

    // Append carbon copy receipients set in the message
    Sequence< OUString > aStringList = xSimpleMailMessage->getCcRecipient();
    sal_Int32 n, nmax = aStringList.getLength();
    for ( n = 0; n < nmax; n++ )
    {
        aBuffer.append("--cc \"" +
                       OUStringToOString(aStringList[n], osl_getThreadTextEncoding()) +
                       "\" ");
    }

    // Append blind carbon copy receipients set in the message
    aStringList = xSimpleMailMessage->getBccRecipient();
    nmax = aStringList.getLength();
    for ( n = 0; n < nmax; n++ )
    {
        aBuffer.append("--bcc \"" +
                       OUStringToOString(aStringList[n], osl_getThreadTextEncoding()) +
                       "\" ");
    }

    // Append subject if set in the message
    if ( !xSimpleMailMessage->getSubject().isEmpty() )
    {
        aBuffer.append("--subject \"" +
                       OUStringToOString(xSimpleMailMessage->getSubject(), osl_getThreadTextEncoding()) +
                       "\" ");
    }

    // Append attachments set in the message
    aStringList = xSimpleMailMessage->getAttachement();
    nmax = aStringList.getLength();
    for ( n = 0; n < nmax; n++ )
    {
        OUString aSystemPath;
        if ( FileBase::E_None == FileBase::getSystemPathFromFileURL(aStringList[n], aSystemPath) )
        {
            aBuffer.append("--attach \"" +
                           OUStringToOString(aSystemPath, osl_getThreadTextEncoding()) +
                           "\" ");
        }
    }

    OString cmd = aBuffer.makeStringAndClear();
    if ( 0 != pclose(popen(cmd.getStr(), "w")) )
    {
        throw ::com::sun::star::uno::Exception("No mail client configured",
            static_cast < XSimpleMailClient * > (this) );
    }
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CmdMailSuppl::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString(COMP_IMPL_NAME);
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CmdMailSuppl::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = Component_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if ( SupportedServicesNames[n] == ServiceName )
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CmdMailSuppl::getSupportedServiceNames(    )
    throw( RuntimeException )
{
    return Component_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
