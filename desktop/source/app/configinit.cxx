
#include "configinit.hxx"
#include "ssoinit.hxx"

#include "desktop.hrc"
#include "app.hxx"

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#include <stdio.h>

#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#include <com/sun/star/configuration/CannotLoadConfigurationException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <drafts/com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <drafts/com/sun/star/configuration/backend/AuthenticationFailedException.hpp>
#include <drafts/com/sun/star/configuration/backend/InvalidAuthenticationMechanismException.hpp>
#include <drafts/com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <drafts/com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <drafts/com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>


// ----------------------------------------------------------------------------

namespace uno           = ::com::sun::star::uno;
namespace lang          = ::com::sun::star::lang;
namespace configuration = ::com::sun::star::configuration;
namespace backend       = drafts::com::sun::star::configuration::backend;
using rtl::OUString;
using uno::UNO_QUERY;

// ----------------------------------------------------------------------------
static char const CONFIGURATION_PROVIDER[]  = "com.sun.star.configuration.ConfigurationProvider";
static char const LOCAL_BACKEND[]           = "com.sun.star.configuration.backend.LocalSingleBackend";
static char const SIMPLE_BACKEND_WRAPPER[]  = "com.sun.star.configuration.backend.OnlineBackend";
static char const OFFLINE_BACKEND_WRAPPER[] = "com.sun.star.configuration.backend.OfflineBackend";

#define CFG_PREFIX "com.sun.star.configuration.bootstrap."
static char const OFFLINE_ENTRY[] = CFG_PREFIX "Offline";
static char const SERVICE_ENTRY[] = CFG_PREFIX "BackendService";
static char const WRAPPER_ENTRY[] = CFG_PREFIX "BackendWrapper";

// ----------------------------------------------------------------------------
#define arraysize( arr ) ( sizeof (arr)/sizeof *(arr) )

typedef uno::Reference< lang::XMultiServiceFactory > ConfigurationProvider;

#define OUSTRING( constascii ) OUString( RTL_CONSTASCII_USTRINGPARAM( constascii ) )

#define OU2O( ustr, enc ) rtl::OUStringToOString( (ustr), RTL_TEXTENCODING_ ## enc )

#define k_PROVIDER OUSTRING( CONFIGURATION_PROVIDER )
#define k_LOCALBE  OUSTRING( LOCAL_BACKEND )
#define k_SIMPLEWRAPPER  OUSTRING( SIMPLE_BACKEND_WRAPPER )
#define k_OFFLINEWRAPPER OUSTRING( OFFLINE_BACKEND_WRAPPER )
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Get a message string securely. There is a fallback string if the resource
// is not available. Adapted from Desktop::GetMsgString()

OUString getMsgString( USHORT nId, char const * aFallBackMsg )
{
    ResMgr* pResMgr = Desktop::GetDesktopResManager();
    if ( !pResMgr || !nId )
        return OUString::createFromAscii(aFallBackMsg);
    else
        return OUString( ResId( nId, pResMgr ));
}
// ----------------------------------------------------------------------------

/// @attention Must be called (directly or indirectly) from within a catch block
static
bool showFallbackMsg( OUString const & sFallbackMsg )
{
    rtl::OUStringBuffer sMsg( CreateErrorMessageForCurrentConfigurationException() );

    sMsg.appendAscii("\n").append( sFallbackMsg );

    if ( Application::IsRemoteServer() )
    {
        rtl::OString aTmpStr = rtl::OUStringToOString( sMsg.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, aTmpStr.getStr() );
        return true;
    }
    else
    {
        WarningBox aMsgBox( NULL, WB_OK_CANCEL | WB_DEF_OK, sMsg.makeStringAndClear() );
        return (aMsgBox.Execute() == RET_OK);
    }
}
// ----------------------------------------------------------------------------

/// @attention Must be called (directly or indirectly) from within a catch block
static
void showOfflineFallbackMsg( ConfigurationProvider & rxOfflineProvider )
{
    OSL_PRECOND( rxOfflineProvider.is(), "Reporting fallback to provider that could not be created" );

    rtl::OUStringBuffer aFallbackMsg( getMsgString(STR_CONFIG_WARN_LOCAL_FALLBACK,
        "StarOffice will continue the startup using your locally stored personal settings.") );

    aFallbackMsg.appendAscii("\n").append( getMsgString(STR_CONFIG_WARN_OFFLINE,
        "The changes you have made to your personal settings will be stored locally and "
        "synchronized the next time you start StarOffice.") );

    if (! showFallbackMsg( aFallbackMsg.makeStringAndClear() ) )
        rxOfflineProvider.clear();
}
// ----------------------------------------------------------------------------

/// @attention Must be called (directly or indirectly) from within a catch block
static
void showLocalFallbackMsg( ConfigurationProvider & rxLocalProvider )
{
    OSL_PRECOND( rxLocalProvider.is(), "Reporting fallback to provider that could not be created" );

    rtl::OUString aFallbackMsg( getMsgString(STR_CONFIG_WARN_LOCAL_FALLBACK,
        "StarOffice will continue the startup using your locally stored personal settings.") );

    if (! showFallbackMsg( aFallbackMsg ) )
        rxLocalProvider.clear();
}
// ----------------------------------------------------------------------------

/** Called after authentication failures to allow re-login
*/
static
sal_Bool relogin()
{
    rtl::OUStringBuffer sMsg( getMsgString( STR_CONFIG_ERR_LOGIN_FAILED,
                                            "Your login to the central configuration was not successful. "
                                            "Either the user name or password is invalid. ") );

    sMsg.appendAscii("\n").append( getMsgString( STR_SSO_RELOGIN, "Please log in again.") );

    ErrorBox aMsgBox( NULL, WB_RETRY_CANCEL | WB_DEF_RETRY, sMsg.makeStringAndClear() );

    if (aMsgBox.Execute() == RET_RETRY)
    {
        return InitSSO( true );
    }
    else
        return false;
}
// ----------------------------------------------------------------------------

/** Creates the normal configuration provider.

    <p> If creation fails because of invalid authentication,
        offers the opportunity to re-login where applicable.
    </p>
*/
static
ConfigurationProvider createDefaultConfigurationProvider( )
{
    uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();

    OSL_ENSURE( xServiceManager.is(),"No ServiceManager set for CreateApplicationConfigurationProvider");

    ConfigurationProvider xProvider;

    if (xServiceManager.is())
    {
    retry:
        try
        {
            xProvider.set( xServiceManager->createInstance(k_PROVIDER),  UNO_QUERY);
        }
        catch (backend::AuthenticationFailedException & e)
        {

            OSL_TRACE( "Configuration: Authentication failed: %s\n",
                        OU2O(e.Message,ASCII_US).getStr() );

            if (!NeedsLogin())
                throw;

            if (!relogin())
            {
                // canceled
                OSL_ASSERT( !xProvider.is() );
                return xProvider;
            }

            goto retry;
        }
    }

    if (!xProvider.is())
    {
        OUString const sMsg = OUSTRING("Service \"") + k_PROVIDER +
                                OUSTRING("\" is not available at the service manager.");

        throw lang::ServiceNotRegisteredException(sMsg, xServiceManager);
    }

    return xProvider;
}
// ----------------------------------------------------------------------------

static
uno::Reference< uno::XComponentContext >
    wrapContext( cppu::ContextEntry_Init * pEntries, sal_Int32 nEntries )
{
    uno::Reference< uno::XComponentContext > xBaseContext;
    try
    {
        uno::Reference< ::com::sun::star::beans::XPropertySet >
            xPS( ::comphelper::getProcessServiceFactory(), UNO_QUERY );

        OSL_ENSURE( xPS.is(), "Cannot get default component context for the process service-manager: no property-set");
        if (xPS.is())
        {
            OSL_VERIFY( xPS->getPropertyValue( OUSTRING( "DefaultContext" ) ) >>= xBaseContext );
        }
    }
    catch (uno::Exception & )
    {
        OSL_ENSURE( false, "Cannot get default component context for the process service-manager");
    }
    return cppu::createComponentContext(pEntries, nEntries, xBaseContext);
}
// ----------------------------------------------------------------------------

static
sal_Bool tryCreateConfigurationWithContext( ConfigurationProvider & rxProvider, uno::Reference< uno::XComponentContext > const & xContext )
{
    OSL_PRECOND( xContext.is(), "NULL context" );
    OSL_PRECOND( !rxProvider.is(), "Provider already set" );

    try
    {
        uno::Reference< lang::XMultiComponentFactory > xFactory = xContext->getServiceManager();
        OSL_ENSURE( xFactory.is(), "Context has no service factory !" );

        if (xFactory.is())
            rxProvider.set( xFactory->createInstanceWithContext( k_PROVIDER, xContext ), UNO_QUERY );
    }
    catch (uno::Exception & )
    {
    }

    return rxProvider.is();
}
// ----------------------------------------------------------------------------

static
inline
cppu::ContextEntry_Init defineContextEntry( OUString const & name, uno::Any const & value)
{
    cppu::ContextEntry_Init entry;
    entry.bLateInitService  = false;
    entry.name              = name;
    entry.value             = value;
    return entry;
}
// ----------------------------------------------------------------------------

static
sal_Bool tryCreateOfflineConfiguration( ConfigurationProvider & rxProvider )
{
    cppu::ContextEntry_Init aEntries[] =
    {
//      { false, OUSTRING( WRAPPER_ENTRY ), uno::makeAny<OUString>( k_OFFLINEWRAPPER ) },
        defineContextEntry( OUSTRING( OFFLINE_ENTRY ), uno::makeAny<sal_Bool>(sal_True) )
    };
    return tryCreateConfigurationWithContext( rxProvider, wrapContext(aEntries, arraysize( aEntries )) );
}
// ----------------------------------------------------------------------------

static
sal_Bool tryCreateLocalConfiguration( ConfigurationProvider & rxProvider )
{
    cppu::ContextEntry_Init aEntries[] =
    {
        defineContextEntry(  OUSTRING( SERVICE_ENTRY ), uno::makeAny<OUString>( k_LOCALBE ) ),
        defineContextEntry(  OUSTRING( WRAPPER_ENTRY ), uno::makeAny<OUString>( k_SIMPLEWRAPPER ) ),
        defineContextEntry(  OUSTRING( OFFLINE_ENTRY ), uno::Any() )
    };
    return tryCreateConfigurationWithContext( rxProvider, wrapContext(aEntries, arraysize( aEntries )) );
}
// ----------------------------------------------------------------------------

uno::Reference< lang::XMultiServiceFactory > CreateApplicationConfigurationProvider( )
{
    uno::Reference< lang::XMultiServiceFactory > xProvider;

    try
    {
        xProvider = createDefaultConfigurationProvider( );
    }
    catch (backend::CannotConnectException & )
    {
        if ( tryCreateOfflineConfiguration(xProvider) )
            showOfflineFallbackMsg( xProvider );

        else if ( tryCreateLocalConfiguration(xProvider) )
            showLocalFallbackMsg( xProvider );

        else
            throw;
    }
    catch (backend::BackendAccessException & )
    {
        if ( tryCreateOfflineConfiguration(xProvider) )
            showOfflineFallbackMsg( xProvider );

        else if ( tryCreateLocalConfiguration(xProvider) )
            showLocalFallbackMsg( xProvider );

        else
            throw;
    }
    catch (uno::Exception & )
    {
        if ( tryCreateLocalConfiguration(xProvider) )
            showLocalFallbackMsg( xProvider );

        else
            throw;
    }

    return xProvider;
}
// ----------------------------------------------------------------------------
OUString CreateErrorMessageForCurrentConfigurationException()
{
    try
    {
        throw;
    }
    catch (configuration::InvalidBootstrapFileException & )
    {
        return getMsgString( STR_CONFIG_ERR_SETTINGS_INCOMPLETE,
                            "The startup settings for accessing the central configuration are incomplete. ");
    }
    catch (backend::InvalidAuthenticationMechanismException & )
    {
        return getMsgString( STR_CONFIG_ERR_MECHANISM_INVALID,
                            "The specified authentication method to access the central configuration is not supported. ");
    }
    catch (backend::AuthenticationFailedException & )
    {
        return getMsgString( STR_CONFIG_ERR_LOGIN_FAILED,
                            "Your login to the central configuration was not successful. "
                            "Either the user name or password is invalid. ");
    }
    catch (backend::CannotConnectException & )
    {
        return getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to the central configuration could not be established. ");
    }
    catch (backend::InsufficientAccessRightsException & )
    {
        return getMsgString( STR_CONFIG_ERR_RIGHTS_MISSING,
                            "You cannot access the central configuration because of missing access rights. ");
    }
    catch (backend::BackendAccessException & )
    {
        return getMsgString( STR_CONFIG_ERR_ACCESS_GENERAL,
                            "A general access error occurred while accessing your central configuration.");
    }
    catch (backend::BackendSetupException & )
    {
        return getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to the central configuration could not be established. ");
    }
    catch (configuration::CannotLoadConfigurationException & )
    {
        return getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to the central configuration could not be established. ");
    }
    catch (uno::Exception & )
    {
        return getMsgString( STR_CONFIG_ERR_ACCESS_GENERAL,
                            "A general error occurred while accessing your central configuration.");
    }
}

// ----------------------------------------------------------------------------
