/*************************************************************************
 *
 *  $RCSfile: shellexec.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:16:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SHELLEXEC_HXX_
#include "shellexec.hxx"
#endif

#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include <errno.h>
#include <unistd.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;
using com::sun::star::beans::PropertyValue;
using com::sun::star::container::NoSuchElementException;
using com::sun::star::container::XNameAccess;

using rtl::OString;
using rtl::OUString;
using rtl::OStringBuffer;
using rtl::OUStringBuffer;
using osl::FileBase;
using osl::MutexGuard;
using osl::Mutex;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define SHELLEXEC_IMPL_NAME  "com.sun.star.comp.system.SystemShellExecute"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{
    Sequence< OUString > SAL_CALL ShellExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii("com.sun.star.sys.shell.SystemShellExecute");
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

ShellExec::ShellExec( const Reference< XMultiServiceFactory >& xServiceManager ) :
    WeakComponentImplHelper3< XSystemShellExecute, XEventListener, XServiceInfo >( m_aMutex ),
    m_xServiceManager( xServiceManager )
{
    try {
        Reference< XComponent > xComponent = Reference< XComponent >( xServiceManager, UNO_QUERY );

        if( xComponent.is() )
            xComponent->addEventListener( static_cast < XEventListener * > (this) );
    }

    catch( ... )
    {
        m_xServiceManager.clear();
        OSL_TRACE( "ShellExec: corrupted service manager given." );
    }
}

//-------------------------------------------------
//
//-------------------------------------------------

void SAL_CALL ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
        throw (IllegalArgumentException, SystemShellExecuteException, RuntimeException)
{
    OString aCommandLine = OUStringToOString( aCommand, osl_getThreadTextEncoding() );

    // check if file exists and is executable -
    //  if not, it is either an url or a document
//    if( 0 != access( aCommandLine.getStr(), X_OK ) )

    // for now, only execute urls
    if( 1 )
    {
        OUString aURL( aCommand );
        OUString aProtocol, aHandler;

        // save this value here
//        int nerr = errno;
        // default error value here
        int nerr = ENOEXEC;

        // find protocol specifier
        sal_Int32 nIndex = aCommand.indexOf( (sal_Unicode) ':' );

        if( nIndex > 0 )
        {
            // FIXME: check for valid protocols here - proposed API for rtl

            // protocol keys are stored in lower case
            aProtocol = aCommand.copy( 0, nIndex ).toAsciiLowerCase();
        }

        if( ( 0 == aProtocol.getLength() ) &&
            ( osl_File_E_None == FileBase::getFileURLFromSystemPath( aCommand, aURL ) ) )
        {
            // seems to be a document
            aProtocol = OUString::createFromAscii( "file" );
        }

        // if aCommand contains a url with a known protocol,
        //  retrieve the registered handler from configuration
        if( aProtocol.getLength() )
        {
            MutexGuard aGuard( m_aMutex );

            // create config manager if not already done so
            if( m_xServiceManager.is() && !m_xConfigurationProvider.is() )
            {
                try
                {
                    m_xConfigurationProvider = Reference< XMultiServiceFactory > (
                        m_xServiceManager->createInstance(
                            OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" ) ),
                        UNO_QUERY );

                    Reference< XComponent > xComponent =
                        Reference< XComponent >( m_xConfigurationProvider, UNO_QUERY );

                    if( xComponent.is() )
                        xComponent->addEventListener( static_cast < XEventListener * > (this) );
                }

                // release service manager instance on runtime exceptions
                catch ( RuntimeException e )
                {
                    m_xServiceManager.clear();
                    OSL_TRACE( "ShellExec: can not instanciate configuration provider." );
                    throw e;
                }
            }

            if( m_xConfigurationProvider.is() )
            {
                try
                {
                    Sequence< Any > aArgumentList( 1 );

                    PropertyValue aProperty;
                    aProperty.Name = OUString::createFromAscii( "nodepath" );
                    aProperty.Value = makeAny( OUString::createFromAscii( "org.openoffice.Office.Common/ExternalApps" ) );

                    aArgumentList[0] = makeAny( aProperty );

                    // query the configured handle for this protocol
                    Reference< XNameAccess > xNameAccess =
                        Reference< XNameAccess > (
                            m_xConfigurationProvider->createInstanceWithArguments(
                                OUString::createFromAscii( "com.sun.star.configuration.ConfigurationAccess" ),
                                aArgumentList ),
                            UNO_QUERY );

                    if( xNameAccess.is() )
                    {
                        // save the registered handler
                        xNameAccess->getByName( aProtocol ) >>= aHandler;
                    }
                }

                catch( NoSuchElementException e )
                {
                    OSL_TRACE( "ShellExec: unknown protocol.\n" );
                    OSL_TRACE( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                }

                catch( WrappedTargetException e )
                {
                    OSL_TRACE( "ShellExec: unexspected exception.\n" );
                    OSL_TRACE( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                }

                catch( RuntimeException e )
                {
                    m_xConfigurationProvider.clear();
                    OSL_TRACE( "ShellExec: configuration provider." );
                    OSL_TRACE( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                    throw e;
                }

                catch( Exception e )
                {
                    OSL_TRACE( "ShellExec: unexspected exception.\n" );
                    OSL_TRACE( OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                }
            }
        }

        if( aHandler.getLength() )
        {
            // search handler in system path if no absolute path given
            FileBase::searchFileURL( aHandler, OUString(), aHandler );

            // handler may be stored as file URL
            FileBase::getSystemPathFromFileURL( aHandler, aHandler );

            // due to a possible convertion to file url,
            // rebuild command line from scratch
            OUStringBuffer aBuffer( aHandler.getLength() + aURL.getLength() + 5 );

            aBuffer.append( (sal_Unicode) '\"' );
            aBuffer.append( aHandler );
            aBuffer.append( (sal_Unicode) '\"' );
            aBuffer.append( (sal_Unicode) ' ' );
            aBuffer.append( (sal_Unicode) '\'' );
            aBuffer.append( aURL );
            aBuffer.append( (sal_Unicode) '\'' );

            aCommandLine = OUStringToOString( aBuffer.makeStringAndClear(), osl_getThreadTextEncoding() );
        }
        else
        {
            // no handler installed
            throw SystemShellExecuteException(
                OUString::createFromAscii( strerror( nerr ) ),
                static_cast < XSystemShellExecute * > (this),
                nerr );
        }
    }

    // append parameter if any
    if( aParameter.getLength() )
    {
        OString aTmp = OUStringToOString( aParameter, osl_getThreadTextEncoding() );

        OStringBuffer aBuffer( aCommandLine.getLength() + aTmp.getLength() + 1 );

        aBuffer.append( aCommandLine );
        aBuffer.append( ' ' );
        aBuffer.append( aTmp );

        aCommandLine = aBuffer.makeStringAndClear();
    }

    // check if the handler really exists and is executable
    OString aHandler = aCommandLine.copy( 1, aCommandLine.indexOf( '\"', 1 ) - 1 );
    if( 0 != access( aHandler.getStr(), X_OK ) )
    {
        int nerr = errno;

        throw SystemShellExecuteException(
            OUString::createFromAscii( strerror( nerr ) ),
            static_cast < XSystemShellExecute * > (this),
            nerr );
    }

    // do not wait for completion
    aCommandLine += " &";

    if( 0 != system( aCommandLine.getStr() ) )
    {
        int nerr = errno;

        throw SystemShellExecuteException(
            OUString::createFromAscii( strerror( nerr ) ),
            static_cast < XSystemShellExecute * > (this),
            nerr );
    }
}

//------------------------------------------------
// XEventListener
//------------------------------------------------

void SAL_CALL ShellExec::disposing( const ::com::sun::star::lang::EventObject& aEvent )
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    if( m_xServiceManager == aEvent.Source )
    {
        m_xServiceManager.clear();
    }

    else if( m_xConfigurationProvider == aEvent.Source )
    {
        m_xConfigurationProvider.clear();
    }
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL ShellExec::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString::createFromAscii( SHELLEXEC_IMPL_NAME );
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

