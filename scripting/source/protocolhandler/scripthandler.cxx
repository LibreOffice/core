/*************************************************************************
*
*  $RCSfile: scripthandler.cxx,v $
*
*  $Revision: 1.14 $
*
*  last change: $Author: dfoster $ $Date: 2003-10-29 15:44:29 $
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

#include "scripthandler.hxx"

#include <osl/mutex.hxx>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <sfx2/objsh.hxx>
#include <sfx2/frame.hxx>

#include <cppuhelper/factory.hxx>
#include <util/util.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::drafts::com::sun::star::script;

namespace scripting_protocolhandler
{

const sal_Char * const MYSERVICENAME = "com.sun.star.frame.ProtocolHandler";
const sal_Char * const MYIMPLNAME = "com.sun.star.comp.ScriptProtocolHandler";
const sal_Char * MYSCHEME = "vnd.sun.star.script:";
const sal_Int32 MYSCHEME_LEN = 20;

void SAL_CALL ScriptProtocolHandler::initialize(
    const css::uno::Sequence < css::uno::Any >& aArguments )
    throw ( css::uno::Exception )
{
    if ( m_bInitialised )
    {
        OSL_TRACE( "ScriptProtocolHandler Already initialised" );
        return ;
    }
    // first argument contains a reference to the frame (may be empty or the desktop,
    // but usually it's a "real" frame)
    if ( sal_False == ( aArguments[ 0 ] >>= m_xFrame ) )
    {
        ::rtl::OUString temp = OUSTR( "ScriptProtocolHandler::initialize: could not extract reference to the frame" );
        throw RuntimeException( temp, Reference< XInterface >() );
    }

    validateXRef( m_xFactory,
        "ScriptProtocolHandler::initialize: No Service Manager available" );
    OSL_TRACE( "ScriptProtocolHandler::initialize\n " );
    m_bInitialised = true;
}

Reference< XDispatch > SAL_CALL ScriptProtocolHandler::queryDispatch(
    const URL& aURL, const ::rtl::OUString& sTargetFrameName, sal_Int32 nSearchFlags )
    throw( ::com::sun::star::uno::RuntimeException )
{
    Reference< XDispatch > xDispatcher;
    OSL_TRACE( "ScriptProtocolHandler::queryDispatch - 1, for URL.complete %s \n",
        ::rtl::OUStringToOString( aURL.Complete, RTL_TEXTENCODING_ASCII_US ).pData->buffer  );

    if ( aURL.Complete.compareToAscii( ::scripting_protocolhandler::MYSCHEME,
                                       ::scripting_protocolhandler::MYSCHEME_LEN ) == 0 )
    {
        xDispatcher = this;
    }
    else
    {
        OSL_TRACE("No match for scheme");
    }
    OSL_TRACE( "ScriptProtocolHandler::queryDispatch - 2\n" );

    return xDispatcher;
}

Sequence< Reference< XDispatch > > SAL_CALL
ScriptProtocolHandler::queryDispatches(
const Sequence < DispatchDescriptor >& seqDescriptor )
throw( RuntimeException )
{
    sal_Int32 nCount = seqDescriptor.getLength();
    Sequence< Reference< XDispatch > > lDispatcher( nCount );
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        lDispatcher[ i ] = this->queryDispatch( seqDescriptor[ i ].FeatureURL,
                                                seqDescriptor[ i ].FrameName,
                                                seqDescriptor[ i ].SearchFlags );
    }
    OSL_TRACE( "ScriptProtocolHandler::queryDispatches \n" );

    return lDispatcher;
}

void SAL_CALL ScriptProtocolHandler::dispatchWithNotification(
    const URL& aURL, const Sequence < PropertyValue >& lArgs,
    const Reference< XDispatchResultListener >& xListener )
    throw ( RuntimeException )
{

    sal_Bool bSuccess = sal_False;
    Any invokeResult;



    OSL_TRACE( "ScriptProtocolHandler::dispatchWithNotification - start \nInput URL %s and %d args\n",
    ::rtl::OUStringToOString( aURL.Complete, RTL_TEXTENCODING_ASCII_US ).pData->buffer, lArgs.getLength() );
    if ( m_bInitialised )
    {
        try
        {
            // obtain the SfxObject shell for our security check
            SfxObjectShell* pDocShell = NULL;
            Reference < XFrame > xFrame( m_xFrame.get(), UNO_QUERY );
            if ( xFrame.is() )
            {
                SfxFrame* pFrame=0;
                for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
                {
                    if ( pFrame->GetFrameInterface() == xFrame )
                        break;
                }

                if ( pFrame )
                    pDocShell = pFrame->GetCurrentDocument();
            }

            // Security check
            pDocShell->AdjustMacroMode( String() );
            OSL_TRACE( "ScriptProtocolHandler::dispatchWithNotification: MacroMode = %d", pDocShell->GetMacroMode() );
            if ( pDocShell->GetMacroMode() == ::com::sun::star::document::MacroExecMode::NEVER_EXECUTE )
            {
                    // check forbids execution
                    ::rtl::OUString temp = OUSTR( "ScriptProtocolHandler::dispatchWithNotification: execution permission denied. " );
                    throw RuntimeException( temp, Reference< XInterface >() );
            }


            // Creates a ScriptProvider ( if one is not created allready )
            createScriptProvider( aURL.Complete );

            Reference< provider::XScript > xFunc =
                m_xScriptProvider->getScript( aURL.Complete );
            validateXRef( xFunc,
                "ScriptProtocolHandler::dispatchWithNotification: validate xFunc - unable to obtain XScript interface" );


            Sequence< Any > inArgs( 0 );
            Sequence< Any > outArgs( 0 );
            Sequence< sal_Int16 > outIndex;

            if ( lArgs.getLength() > 0 )
            {
               int argCount = 0;
               for ( int index = 0; index < lArgs.getLength(); index++ )
               {
                   OSL_TRACE(" processing arg %d, name %s of type %s", index,
                       ::rtl::OUStringToOString(
                           lArgs[ index ].Name,
                           RTL_TEXTENCODING_ASCII_US ).pData->buffer,
                       ::rtl::OUStringToOString(
                           lArgs[ index ].Value.getValueTypeName(),
                           RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                   // Sometimes we get a propertyval with name = "Referer"
                   // this is not an argument to be passed to script, so
                   // ignore.
                   if ( lArgs[ index ].Name.compareToAscii("Referer") != 0  ||
                        lArgs[ index ].Name.getLength() == 0 )
                   {
                       inArgs.realloc( ++argCount );
                       OSL_TRACE("Adding arg to inArgs");
                       inArgs[ index ] = lArgs[ index ].Value;
                   }
               }
            }
            invokeResult = xFunc->invoke( inArgs, outIndex, outArgs );
            bSuccess = sal_True;
        }

        // Office doesn't handle exceptions rethrown here very well, it cores,
        // all we can is log them and then set fail for the dispatch event!
        // (if there is a listener of course)
        catch ( RuntimeException & e )
        {
            OSL_TRACE( "ScriptProtocolHandler::dispatchWithNotificationn caught RuntimeException: %s",
            ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer
            );
            ::rtl::OUString reason = ::rtl::OUString::createFromAscii(
            "ScriptProtocolHandler::dispatchWithNotification: caught RuntimeException: "
            );
            reason.concat( e.Message );
            invokeResult <<= reason;
        }
        catch ( Exception & e )
        {
            OSL_TRACE( "ScriptProtocolHandler::dispatchWithNotificationn caught Exception: %s",
            ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer
            );
            ::rtl::OUString reason = ::rtl::OUString::createFromAscii(
            "ScriptProtocolHandler::dispatchWithNotification: caught  Exception: "
            );
            reason.concat( e.Message );
            invokeResult <<= reason;
        }
#ifdef _DEBUG
        catch ( ... )
        {
            ::rtl::OUString reason = ::rtl::OUString::createFromAscii(
            "ScriptProtocolHandler::dispatchWithNotification: caught unknown exception "
            );
            invokeResult <<= reason;
        }
#endif

    }
    else
    {
        OSL_TRACE( "ScriptProtocolHandler::dispatchWithNotification: failed, ScriptProtocolHandler not initialised" );
        ::rtl::OUString reason = ::rtl::OUString::createFromAscii(
        "ScriptProtocolHandler::dispatchWithNotification failed, ScriptProtocolHandler not initialised"
        );
        invokeResult <<= reason;
    }

    if ( xListener.is() )
    {
        // always call dispatchFinished(), because we didn't load a document but
        // executed a macro instead!
        ::com::sun::star::frame::DispatchResultEvent aEvent;

        aEvent.Source = static_cast< ::cppu::OWeakObject* >( this );
        aEvent.Result = invokeResult;
        if ( bSuccess )
        {
            aEvent.State = ::com::sun::star::frame::DispatchResultState::SUCCESS;
        }
        else
        {
            aEvent.State = ::com::sun::star::frame::DispatchResultState::FAILURE;
        }

        try
        {
            xListener->dispatchFinished( aEvent ) ;
        }
        catch(RuntimeException & e)
        {
            OSL_TRACE(
            "ScriptProtocolHandler::dispatchWithNotification: caught RuntimeException"
            "while dispatchFinished %s",
            ::rtl::OUStringToOString( e.Message,
            RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        }
    }
    OSL_TRACE( "ScriptProtocolHandler::dispatchWithNotification - end" );

}

void SAL_CALL ScriptProtocolHandler::dispatch(
const URL& aURL, const Sequence< PropertyValue >& lArgs )
throw ( RuntimeException )
{
    OSL_TRACE("ScriptProtocolHandler::dispatch");
    dispatchWithNotification( aURL, lArgs, Reference< XDispatchResultListener >() );
}

void SAL_CALL ScriptProtocolHandler::addStatusListener(
const Reference< XStatusListener >& xControl, const URL& aURL )
throw ( RuntimeException )
{
    // implement if status is supported
}

void SAL_CALL ScriptProtocolHandler::removeStatusListener(
const Reference< XStatusListener >& xControl, const URL& aURL )
throw ( RuntimeException )
{}

void
ScriptProtocolHandler::createScriptProvider( const ::rtl::OUString& url )
throw ( RuntimeException )
{
    if ( m_xScriptProvider.is() )
    {
        OSL_TRACE("ScriptProtocolHandler::createScriptProvider(), function provider already created");
        return;
    }
    try
    {
        css::uno::Sequence < css::uno::Any > args( 1 );
        Reference< XModel > xModel;
        if ( m_xFrame.is() )
        {
            Reference< XController > xController = m_xFrame->getController();
            if ( xController .is() )
            {
                xModel = xController->getModel();
            }
        }

        rtl::OUString documentString = rtl::OUString::createFromAscii( "location=document" );
        rtl::OUString filesystemString = rtl::OUString::createFromAscii( "location=filesystem" );

        // Detect if workaround is necessary.
        // Problem, when ScriptProvider is created,
        // and document contains scripts, storage mangager adds script
        // storage to security manager ( this results in security dialogs
        // getting raised. ) This is a problem as ScriptProvider is not
        // at this time created by the document and dialogs are raised
        // at unexpected times. This code should be removed when provider
        // is created by document.
        // Workaround: If uri of script to be invoked is NOT a document
        // located script create ScriptProvider with extra paramater
        // which indicates to storage not to use security
        //
        // workaround also applies to scripts located on the filesystem
        if ( ( url.indexOf( documentString ) == -1 ) && ( url.indexOf( filesystemString ) == -1 ) )
        {
            // Not a document or filesystem script - no need to use security
            OSL_TRACE(" Will create special ScriptProvider eg. one that doesn't user security" );
            args.realloc( 2 );
            args[ 1 ] <<= sal_False;
        }
        args[ 0 ] <<= xModel;
        Reference< XInterface > xXinterface =
        m_xFactory->createInstanceWithArguments(
            ::rtl::OUString::createFromAscii(
            "drafts.com.sun.star.script.provider.MasterScriptProvider" ),
            args );
        validateXRef( xXinterface,
            "ScriptProtocolHandler::initialize: cannot get instance of MasterScriptProvider" );
        m_xScriptProvider = Reference< provider::XScriptProvider >( xXinterface,
            UNO_QUERY_THROW );
    }
    catch ( RuntimeException & e )
    {
        ::rtl::OUString temp = OUSTR( "ScriptProtocolHandler::createScriptProvider(),  " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
    catch ( Exception & e )
    {
        OSL_TRACE( "ScriptProtocolHandler::createScriptProvider: Caught Exception %s",
        ::rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        ::rtl::OUString temp = OUSTR( "ScriptProtocolHandler::createScriptProvider: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
#ifdef _DEBUG
    catch ( ... )
    {
        OSL_TRACE( "ScriptProtocolHandler::createScriptProvier: Unknown exception caught" );
        throw RuntimeException(
        OUSTR( "ScriptProtocolHandler::createScriptProvider: UnknownException: " ),
            Reference< XInterface > () );
    }
#endif

}

ScriptProtocolHandler::ScriptProtocolHandler(
Reference< css::lang::XMultiServiceFactory > const& rFact ) :
m_bInitialised( false ), m_xFactory( rFact )
{
    OSL_TRACE( "ScriptProtocolHandler::ScriptProtocolHandler - ctor" );
}

ScriptProtocolHandler::~ScriptProtocolHandler()
{
    OSL_TRACE( "ScriptProtocolHandler::ScriptProtocolHandler - dtor" );
}

/* XServiceInfo */
::rtl::OUString SAL_CALL ScriptProtocolHandler::getImplementationName( )
throw( RuntimeException )
{
    return impl_getStaticImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL ScriptProtocolHandler::supportsService(
const ::rtl::OUString& sServiceName )
throw( RuntimeException )
{
    Sequence< ::rtl::OUString > seqServiceNames = getSupportedServiceNames();
    const ::rtl::OUString* pArray = seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter = 0; nCounter < seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[ nCounter ] == sServiceName )
        {
            return sal_True ;
        }
    }

    return sal_False ;
}

/* XServiceInfo */
Sequence< ::rtl::OUString > SAL_CALL ScriptProtocolHandler::getSupportedServiceNames()
throw( RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
Sequence< ::rtl::OUString > ScriptProtocolHandler::impl_getStaticSupportedServiceNames()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    Sequence< ::rtl::OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [ 0 ] =
        ::rtl::OUString::createFromAscii( ::scripting_protocolhandler::MYSERVICENAME );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
::rtl::OUString ScriptProtocolHandler::impl_getStaticImplementationName()
{
    return ::rtl::OUString::createFromAscii( ::scripting_protocolhandler::MYIMPLNAME );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL ScriptProtocolHandler::impl_createInstance(
const Reference< css::lang::XMultiServiceFactory >& xServiceManager )
throw( RuntimeException )
{
    return Reference< XInterface > ( *new ScriptProtocolHandler( xServiceManager ) );
}

/* Factory for registration */
Reference< XSingleServiceFactory > ScriptProtocolHandler::impl_createFactory(
const Reference< XMultiServiceFactory >& xServiceManager )
{
    Reference< XSingleServiceFactory > xReturn (
        cppu::createSingleFactory( xServiceManager,
            ScriptProtocolHandler::impl_getStaticImplementationName(),
            ScriptProtocolHandler::impl_createInstance,
            ScriptProtocolHandler::impl_getStaticSupportedServiceNames() )
    );
    return xReturn;
}

} // namespace scripting_protocolhandler

/* exported functions for registration */
extern "C"
{

    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char** ppEnvironmentTypeName, uno_Environment** ppEnvironment )
    {
        *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
    }

    sal_Bool SAL_CALL component_writeInfo( void * pServiceManager ,
                                           void * pRegistryKey )
    {
        Reference< css::registry::XRegistryKey > xKey(
            reinterpret_cast< css::registry::XRegistryKey* >( pRegistryKey ) ) ;

        ::rtl::OUString aStr = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
        aStr +=
            ::scripting_protocolhandler::ScriptProtocolHandler::impl_getStaticImplementationName();

        aStr += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES" ) );
        Reference< css::registry::XRegistryKey > xNewKey = xKey->createKey( aStr );
        xNewKey->createKey(
            ::rtl::OUString::createFromAscii( ::scripting_protocolhandler::MYSERVICENAME )
            );

        return sal_True;
    }

    void* SAL_CALL component_getFactory( const sal_Char * pImplementationName ,
                                         void * pServiceManager ,
                                         void * pRegistryKey )
    {
        // Set default return value for this operation - if it failed.
        void * pReturn = NULL ;

        if (
            ( pImplementationName != NULL ) &&
            ( pServiceManager != NULL )
        )
        {
            // Define variables which are used in following macros.
            ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XSingleServiceFactory > xFactory ;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
            xServiceManager( reinterpret_cast<
            ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ) ) ;

            if ( ::scripting_protocolhandler::ScriptProtocolHandler::impl_getStaticImplementationName().equals(
                ::rtl::OUString::createFromAscii( pImplementationName ) ) )
            {
                xFactory = ::scripting_protocolhandler::ScriptProtocolHandler::impl_createFactory( xServiceManager );
            }

            // Factory is valid - service was found.
            if ( xFactory.is() )
            {
                xFactory->acquire();
                pReturn = xFactory.get();
            }
        }

        // Return with result of this operation.
        return pReturn ;
    }
} // extern "C"


