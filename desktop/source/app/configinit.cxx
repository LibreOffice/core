
#include "configinit.hxx"
//#include "ssoinit.hxx"

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
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
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
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#include <com/sun/star/configuration/CannotLoadConfigurationException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/AuthenticationFailedException.hpp>
#include <com/sun/star/configuration/backend/InvalidAuthenticationMechanismException.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>

// ----------------------------------------------------------------------------

namespace uno           = ::com::sun::star::uno;
namespace lang          = ::com::sun::star::lang;
namespace beans         = ::com::sun::star::beans;
namespace util          = ::com::sun::star::util;
namespace task          = ::com::sun::star::task;
namespace configuration = ::com::sun::star::configuration;
namespace backend       = ::com::sun::star::configuration::backend;
using rtl::OUString;
using uno::UNO_QUERY;
using desktop::Desktop;

// ----------------------------------------------------------------------------
static char const CONFIGURATION_PROVIDER[]  = "com.sun.star.configuration.ConfigurationProvider";
static char const LOCAL_BACKEND[]           = "com.sun.star.configuration.backend.LocalSingleBackend";
static char const SIMPLE_BACKEND_WRAPPER[]  = "com.sun.star.configuration.backend.OnlineBackend";

static char const READONLY_ACCESS[]         = "com.sun.star.configuration.ConfigurationAccess";
static char const UPDATE_ACCESS[]           = "com.sun.star.configuration.ConfigurationUpdateAccess";


#define CFG_PREFIX "/modules/com.sun.star.configuration/bootstrap/"
#define CFG_INIPREFIX "CFG_"
static char const SERVICE_ENTRY[] = CFG_PREFIX "BackendService";
static char const WRAPPER_ENTRY[] = CFG_PREFIX "BackendWrapper";


#define CONTEXT_ITEM_PASSTHRU "/implementations/com.sun.star.com.configuration.bootstrap.ComponentContext/isPassthrough"

// ----------------------------------------------------------------------------
#define arraysize( arr ) ( sizeof (arr)/sizeof *(arr) )

typedef uno::Reference< lang::XMultiServiceFactory > ConfigurationProvider;

#define OUSTRING( constascii ) OUString( RTL_CONSTASCII_USTRINGPARAM( constascii ) )

#define OU2O( ustr, enc ) rtl::OUStringToOString( (ustr), RTL_TEXTENCODING_ ## enc )

#define k_PROVIDER OUSTRING( CONFIGURATION_PROVIDER )
#define k_LOCALBE  OUSTRING( LOCAL_BACKEND )
#define k_SIMPLEWRAPPER  OUSTRING( SIMPLE_BACKEND_WRAPPER )
// ----------------------------------------------------------------------------


static uno::Reference< uno::XInterface > getConfigurationSettings( ConfigurationProvider const & xProvider, bool bUpdate );

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

void setMsgBoxTitle( MessBox & aMsgBox )
{
    ResMgr* pResMgr = Desktop::GetDesktopResManager();

    OUString aMsgBoxTitle = pResMgr ?   OUString( ResId( STR_TITLE_CONFIG_MSGBOX, pResMgr )) :
                                        utl::Bootstrap::getProductKey();

    if (aMsgBoxTitle.getLength())
        aMsgBox.SetText(aMsgBoxTitle);
}
// ----------------------------------------------------------------------------

static
bool showFallbackMsg( OUString const & sFallbackMsg,
                     const rtl::OUString& aMessage)
{
    rtl::OUStringBuffer sMsg(aMessage);

    sMsg.appendAscii("\n").append( sFallbackMsg );

    WarningBox aMsgBox( NULL, WB_OK_CANCEL | WB_DEF_OK, sMsg.makeStringAndClear() );
    setMsgBoxTitle( aMsgBox );
    return (aMsgBox.Execute() == RET_OK);
}
// ----------------------------------------------------------------------------

static
void showLocalFallbackMsg( ConfigurationProvider & rxLocalProvider,
                            const rtl::OUString& aMessage)
{
    OSL_PRECOND( rxLocalProvider.is(), "Reporting fallback to provider that could not be created" );

    rtl::OUString aFallbackMsg( getMsgString(STR_CONFIG_WARN_LOCAL_FALLBACK,
        "StarOffice will continue the startup using only your locally stored personal settings.") );

    if (! showFallbackMsg( aFallbackMsg, aMessage ) )
        rxLocalProvider.clear();
}
// ----------------------------------------------------------------------------
static
uno::Reference< uno::XComponentContext > getProcessContext( )
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
    return xBaseContext;
}
// ----------------------------------------------------------------------------

static
inline
uno::Reference< uno::XComponentContext >
    wrapContext( cppu::ContextEntry_Init * pEntries, sal_Int32 nEntries )
{
    uno::Reference< uno::XComponentContext > xBaseContext = getProcessContext( );

    return cppu::createComponentContext(pEntries, nEntries, xBaseContext);
}
// ----------------------------------------------------------------------------

/** Creates the normal configuration provider.

*/
static
ConfigurationProvider createDefaultConfigurationProvider( )
{
    uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();

    OSL_ENSURE( xServiceManager.is(),"No ServiceManager set for CreateApplicationConfigurationProvider");

    ConfigurationProvider xProvider;

    if (xServiceManager.is())
    {

            xProvider.set( xServiceManager->createInstance(k_PROVIDER),  UNO_QUERY);
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
sal_Bool tryCreateLocalConfiguration( ConfigurationProvider & rxProvider )
{
    cppu::ContextEntry_Init aEntries[] =
    {
        defineContextEntry(  OUSTRING( SERVICE_ENTRY ), uno::makeAny<OUString>( k_LOCALBE ) ),
        defineContextEntry(  OUSTRING( WRAPPER_ENTRY ), uno::makeAny<OUString>( k_SIMPLEWRAPPER ) ),
        // defineContextEntry(  OUSTRING( OFFLINE_ENTRY ), uno::Any() ),
        defineContextEntry(  OUSTRING( CONTEXT_ITEM_PASSTHRU ), uno::makeAny<sal_Bool>(sal_True) )
    };
    return tryCreateConfigurationWithContext( rxProvider, wrapContext(aEntries, arraysize( aEntries )) );
}
// ----------------------------------------------------------------------------
/// @attention this method must be called from a catch statement!
static void handleGeneralException(ConfigurationProvider& xProvider,
                                   uno::Exception& aException,
                                   const rtl::OUString& aMessage)
{
    aException.Message = aMessage ;
    if (tryCreateLocalConfiguration(xProvider))
    {
        showLocalFallbackMsg(xProvider, aMessage) ;
    }
    else { throw ; }
}
// ----------------------------------------------------------------------------

uno::Reference< lang::XMultiServiceFactory > CreateApplicationConfigurationProvider( )
{
    uno::Reference< lang::XMultiServiceFactory > xProvider;

    try
    {
        xProvider = createDefaultConfigurationProvider( );
    }
    catch (configuration::InvalidBootstrapFileException & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_SETTINGS_INCOMPLETE,
                            "The startup settings for your configuration settings are incomplete. "));
    }
     catch (backend::CannotConnectException & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to your configuration settings could not be established. "));
    }

    catch (backend::BackendSetupException & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to your configuration settings could not be established. "));
    }
    catch (configuration::CannotLoadConfigurationException & exception)
    {
        handleGeneralException(xProvider,  exception,
                getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to your configuration settings could not be established. "));
    }
    catch (uno::Exception & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_ACCESS_GENERAL,
                            "A general error occurred while accessing your configuration settings."));
    }


    return xProvider ;
}
// ----------------------------------------------------------------------------





