/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#include "syscmdmail.hxx"
#include "cmdmailmsg.hxx"

#include <com/sun/star/system/MailClientFlags.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>

#include <unistd.h>

using com::sun::star::beans::PropertyValue;
using com::sun::star::container::XNameAccess;
using com::sun::star::system::XMailClient;
using com::sun::star::system::XMailMessage;
using rtl::OString;
using rtl::OStringBuffer;
using rtl::OUString;
using rtl::OUStringToOString;

using namespace com::sun::star::lang;
using namespace com::sun::star::system::MailClientFlags;
using namespace com::sun::star::uno;

#define COMP_SERVICE_NAME  "com.sun.star.system.SystemMailProvider"
#define COMP_IMPL_NAME     "com.sun.star.comp.system.unx.SystemMailProvider"

namespace shell
{

namespace
{
    static void escapeDoubleQuotes( OStringBuffer &rBuffer,
                                    const OUString &ustr,
                                    rtl_TextEncoding iEncoding )
    {
        const OString rStr( OUStringToOString( ustr, iEncoding ) );
        sal_Int32 nIndex = rStr.indexOf('"');
        if ( nIndex == -1 )
            rBuffer.append( rStr );
        else
        {
            const sal_Char *pStart = rStr.getStr();
            const sal_Char *pFrom = pStart;
            const sal_Int32 nLen = rStr.getLength();
            sal_Int32 nPrev = 0;;
            do
            {
                rBuffer.append( pFrom, nIndex - nPrev );
                rBuffer.append( RTL_CONSTASCII_STRINGPARAM( "\\\"" ) );
                nIndex++;
                pFrom = pStart + nIndex;
                nPrev = nIndex;
            }
            while ( ( nIndex = rStr.indexOf( '"' , nIndex ) ) != -1  );

            rBuffer.append( pFrom, nLen - nPrev );
        }
    }
}

SystemCommandMail::SystemCommandMail(
    const Reference< XComponentContext >& xContext )
    : SystemCommandMail_Base( m_aMutex )
    , m_xContext( xContext )
{
    try
    {
        m_xConfigurationProvider.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationProvider") ),
                m_xContext ),
            UNO_QUERY );
    }
    catch(...){}
}

SystemCommandMail::~SystemCommandMail()
{
    m_xConfigurationProvider.clear();
    m_xContext.clear();
}

Reference< XMailClient > SAL_CALL
SystemCommandMail::queryMailClient()
throw ( RuntimeException )
{
    return Reference< XMailClient >(
        static_cast < cppu::OWeakObject * >( this ), UNO_QUERY );
}


Reference< XMailMessage > SAL_CALL
SystemCommandMail::createMailMessage()
throw ( RuntimeException )
{
    return Reference< XMailMessage >(
        static_cast< cppu::OWeakObject *>(
            new CmdMailMsg() ),
        UNO_QUERY );
}


void SAL_CALL
SystemCommandMail::sendMailMessage(
    const Reference< XMailMessage >& xMailMessage,
    sal_Int32 /*aFlag*/ )
throw (IllegalArgumentException, Exception, RuntimeException)
{
    osl::ClearableMutexGuard aGuard( m_aMutex );
    Reference< XMultiServiceFactory > xConfigurationProvider = m_xConfigurationProvider;
    aGuard.clear();

    if ( ! xMailMessage.is() )
    {
        throw IllegalArgumentException(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "No message specified" )),
            static_cast < XMailClient * > (this), 1 );
    }

    if( ! xConfigurationProvider.is() )
    {
        throw Exception(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "Can not access configuration" )),
            static_cast < XMailClient * > (this) );
    }

    OStringBuffer aBuffer;
    aBuffer.append("\"");

    OUString aProgramURL(RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/program/senddoc"));
    rtl::Bootstrap::expandMacros(aProgramURL);
    OUString aProgram;
    if ( osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(aProgramURL, aProgram))
    {
        throw Exception(
            OUString(RTL_CONSTASCII_USTRINGPARAM("Cound not convert executable path")),
            static_cast < XMailClient * > (this));
    }

    const rtl_TextEncoding iEncoding = osl_getThreadTextEncoding();
    aBuffer.append(OUStringToOString(aProgram, iEncoding));
    aBuffer.append("\" ");

    try
    {
        // Query XNameAccess interface of the org.openoffice.Office.Common/ExternalMailer
        // configuration node to retriece the users preferred email application. This may
        // transparently by redirected to e.g. the corresponding GConf setting in GNOME.
        OUString aConfigRoot = OUString(
            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Office.Common/ExternalMailer" ) );

        PropertyValue aProperty;
        aProperty.Name = OUString( RTL_CONSTASCII_USTRINGPARAM("nodepath" ));
        aProperty.Value = makeAny( aConfigRoot );

        Sequence< Any > aArgumentList( 1 );
        aArgumentList[0] = makeAny( aProperty );

        Reference< XNameAccess > xNameAccess =
            Reference< XNameAccess > (
                xConfigurationProvider->createInstanceWithArguments(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" )),
                    aArgumentList ),
                UNO_QUERY );

        if( xNameAccess.is() )
        {
            OUString aMailer;

            // Retrieve the value for "Program" node and append it feed senddoc with it
            // using the (undocumented) --mailclient switch
            xNameAccess->getByName( OUString( RTL_CONSTASCII_USTRINGPARAM("Program") ) ) >>= aMailer;

            if( aMailer.getLength() )
            {
                // make sure we have a system path
                osl::FileBase::getSystemPathFromFileURL( aMailer, aMailer );

                aBuffer.append("--mailclient ");
                aBuffer.append(OUStringToOString( aMailer, iEncoding ));
                aBuffer.append(" ");
            }
#ifdef MACOSX
            else
                aBuffer.append("--mailclient Mail ");
#endif
        }

    }
    catch( RuntimeException e )
    {
        OSL_TRACE( "RuntimeException caught accessing configuration provider." );
        OSL_TRACE( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        throw e;
    }

    // Append body if set in the message
    OUString ustr = xMailMessage->getBody();
    if ( ustr.getLength() > 0 )
    {
        aBuffer.append("--body \"");
        escapeDoubleQuotes( aBuffer, ustr, iEncoding );
        aBuffer.append("\" ");
    }

    // Append subject if set in the message
    ustr = xMailMessage->getSubject();
    if ( ustr.getLength() > 0 )
    {
        aBuffer.append("--subject \"");
        escapeDoubleQuotes( aBuffer, ustr, iEncoding );
        aBuffer.append("\" ");
    }

    // Append originator if set in the message
    if ( xMailMessage->getOriginator().getLength() > 0 )
    {
        aBuffer.append("--from \"");
        aBuffer.append(OUStringToOString(xMailMessage->getOriginator(), iEncoding));
        aBuffer.append("\" ");
    }

    // Append receipient if set in the message
    if ( xMailMessage->getRecipient().getLength() > 0 )
    {
        aBuffer.append("--to \"");
        aBuffer.append(OUStringToOString(xMailMessage->getRecipient(), iEncoding));
        aBuffer.append("\" ");
    }

    // Append carbon copy receipients set in the message
    Sequence< OUString > aStringList = xMailMessage->getCcRecipient();
    sal_Int32 n, nmax = aStringList.getLength();
    for ( n = 0; n < nmax; n++ )
    {
        aBuffer.append("--cc \"");
        aBuffer.append(OUStringToOString(aStringList[n], iEncoding));
        aBuffer.append("\" ");
    }

    // Append blind carbon copy receipients set in the message
    aStringList = xMailMessage->getBccRecipient();
    nmax = aStringList.getLength();
    for ( n = 0; n < nmax; n++ )
    {
        aBuffer.append("--bcc \"");
        aBuffer.append(OUStringToOString(aStringList[n], iEncoding));
        aBuffer.append("\" ");
    }

    // Append attachments set in the message
    aStringList = xMailMessage->getAttachement();
    nmax = aStringList.getLength();
    for ( n = 0; n < nmax; n++ )
    {
        OUString aSystemPath;
        if ( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(aStringList[n], aSystemPath) )
        {
            aBuffer.append("--attach \"");
            aBuffer.append(OUStringToOString(aSystemPath, iEncoding));
            aBuffer.append("\" ");
        }
    }

    OString cmd = aBuffer.makeStringAndClear();
    if ( 0 != pclose(popen(cmd.getStr(), "w")) )
    {
        throw ::com::sun::star::uno::Exception(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "No mail client configured" )),
            static_cast < XMailClient * > (this) );
    }
}

OUString SAL_CALL
SystemCommandMail::getImplementationName(  )
throw( RuntimeException )
{
    return getImplementationName_static();
}

sal_Bool SAL_CALL
SystemCommandMail::supportsService(
    const OUString& ServiceName )
throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = getSupportedServiceNames_static();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

Sequence< OUString > SAL_CALL
SystemCommandMail::getSupportedServiceNames()
throw( RuntimeException )
{
    return getSupportedServiceNames_static();
}

Reference< XInterface >
SystemCommandMail::Create(
    const Reference< XComponentContext > &xContext)
{
    return Reference< XInterface >(
        static_cast< cppu::OWeakObject *>(
            new SystemCommandMail( xContext ) ) );
}

OUString
SystemCommandMail::getImplementationName_static()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( COMP_IMPL_NAME ) );
}

Sequence< OUString >
SystemCommandMail::getSupportedServiceNames_static()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( COMP_SERVICE_NAME ) );
    return aRet;
}

}
