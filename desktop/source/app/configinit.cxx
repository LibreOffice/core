
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
static char const OFFLINE_BACKEND_WRAPPER[] = "com.sun.star.configuration.backend.OfflineBackend";

static char const READONLY_ACCESS[]         = "com.sun.star.configuration.ConfigurationAccess";
static char const UPDATE_ACCESS[]           = "com.sun.star.configuration.ConfigurationUpdateAccess";
static char const USERDATA_LOCATOR[]        = "com.sun.star.configuration.backend.LocalHierarchyBrowser";
static char const USERDATA_IMPORTER[]       = "com.sun.star.configuration.backend.LocalDataImporter";
static char const USERDATA_IMPORTSERVICE[]  = "com.sun.star.configuration.backend.CopyImporter";

static char const CONFIGURATION_SETTINGS[]  = "/org.openoffice.Setup/Configuration";
    static char const SETTING_DOIMPORT[]    = "TransferUserSettingsOnce";

#define CFG_PREFIX "/modules/com.sun.star.configuration/bootstrap/"
#define CFG_INIPREFIX "CFG_"
static char const OFFLINE_ENTRY[] = CFG_PREFIX "Offline";
static char const SERVICE_ENTRY[] = CFG_PREFIX "BackendService";
static char const WRAPPER_ENTRY[] = CFG_PREFIX "BackendWrapper";

static char const INITUSERDATA_ENTRY[] = CFG_INIPREFIX "InitializeUserDataFromURL";

#define CONTEXT_ITEM_PASSTHRU "/implementations/com.sun.star.com.configuration.bootstrap.ComponentContext/isPassthrough"

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

static void initializeUserData( ConfigurationProvider const & xProvider );
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

    if ( Application::IsRemoteServer() )
    {
        rtl::OString aTmpStr = rtl::OUStringToOString( sMsg.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, aTmpStr.getStr() );
        return true;
    }
    else
    {
        WarningBox aMsgBox( NULL, WB_OK_CANCEL | WB_DEF_OK, sMsg.makeStringAndClear() );
        setMsgBoxTitle( aMsgBox );
        return (aMsgBox.Execute() == RET_OK);
    }
}
// ----------------------------------------------------------------------------

static
void showOfflineFallbackMsg( ConfigurationProvider & rxOfflineProvider,
                            const rtl::OUString& aMessage)
{
    OSL_PRECOND( rxOfflineProvider.is(), "Reporting fallback to provider that could not be created" );

    rtl::OUStringBuffer aFallbackMsg( getMsgString(STR_CONFIG_WARN_LOCAL_FALLBACK,
        "StarOffice will continue the startup using your locally stored personal settings.") );

    aFallbackMsg.appendAscii("\n").append( getMsgString(STR_CONFIG_WARN_OFFLINE,
        "The changes you have made to your personal settings will be stored locally and "
        "synchronized the next time you start StarOffice.") );

    if (! showFallbackMsg( aFallbackMsg.makeStringAndClear(), aMessage ) )
        rxOfflineProvider.clear();
}
// ----------------------------------------------------------------------------

static
void showLocalFallbackMsg( ConfigurationProvider & rxLocalProvider,
                            const rtl::OUString& aMessage)
{
    OSL_PRECOND( rxLocalProvider.is(), "Reporting fallback to provider that could not be created" );

    rtl::OUString aFallbackMsg( getMsgString(STR_CONFIG_WARN_LOCAL_FALLBACK,
        "StarOffice will continue the startup using your locally stored personal settings.") );

    if (! showFallbackMsg( aFallbackMsg, aMessage ) )
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
    setMsgBoxTitle( aMsgBox );

    if (aMsgBox.Execute() == RET_RETRY)
    {
        return InitSSO( true );
    }
    else
        return false;
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

    initializeUserData(xProvider);

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
sal_Bool tryCreateOfflineConfiguration( ConfigurationProvider & rxProvider )
{
    cppu::ContextEntry_Init aEntries[] =
    {
//      { false, OUSTRING( WRAPPER_ENTRY ), uno::makeAny<OUString>( k_OFFLINEWRAPPER ) },
        defineContextEntry( OUSTRING( OFFLINE_ENTRY ), uno::makeAny<sal_Bool>(sal_True) ),
        defineContextEntry( OUSTRING( CONTEXT_ITEM_PASSTHRU ), uno::makeAny<sal_Bool>(sal_True) )
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
        defineContextEntry(  OUSTRING( OFFLINE_ENTRY ), uno::Any() ),
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
/// @attention this method must be called from a catch statement!
static void handleAccessException(ConfigurationProvider& xProvider,
                                  uno::Exception& aException,
                                  const rtl::OUString& aMessage)
{
    aException.Message = aMessage ;
    if (tryCreateOfflineConfiguration(xProvider))
    {
        showOfflineFallbackMsg(xProvider, aMessage) ;
    }
    else if (tryCreateLocalConfiguration(xProvider))
    {
        showLocalFallbackMsg(xProvider, aMessage) ;
    }
    else { throw ; }
}
// ----------------------------------------------------------------------------
/// @attention this method must be called from a catch statement!
static void handleConnectException(ConfigurationProvider& xProvider,
                                   uno::Exception& aException,
                                   const rtl::OUString& aMessage)
{
    handleAccessException(xProvider, aException, aMessage) ;
}
// ----------------------------------------------------------------------------

/**
  [cm122549]
  Ok, I know it looks ugly to have the whole list of exceptions being
  caught here in one go, when we have only three different kinds of
  behaviour for them. The problem is that there is apparently a bug in
  Windows which, if we try to do another throw later to refine our
  understanding of the exception, will actually delete the exception
  object twice. Which is not nice, especially when considering what
  store in those exceptions. Hence the catchfest here once and for
  all to be able to generate all the different messages.
  */
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
                            "The startup settings for accessing the central configuration are incomplete. "));
    }
    catch (backend::InvalidAuthenticationMechanismException & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_MECHANISM_INVALID,
                            "The specified authentication method to access the central configuration is not supported. "));
    }
    catch (backend::AuthenticationFailedException & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_LOGIN_FAILED,
                            "Your login to the central configuration was not successful. "
                            "Either the user name or password is invalid. "));
    }
    catch (backend::CannotConnectException & exception)
    {
        handleConnectException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to the central configuration could not be established. "));
    }
    catch (backend::InsufficientAccessRightsException & exception)
    {
        handleAccessException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_RIGHTS_MISSING,
                            "You cannot access the central configuration because of missing access rights. "));
    }
    catch (backend::BackendAccessException & exception)
    {
        handleAccessException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_ACCESS_GENERAL,
                            "A general access error occurred while accessing your central configuration."));
    }
    catch (backend::BackendSetupException & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to the central configuration could not be established. "));
    }
    catch (configuration::CannotLoadConfigurationException & exception)
    {
        handleGeneralException(xProvider,  exception,
                getMsgString( STR_CONFIG_ERR_CANNOT_CONNECT,
                            "A connection to the central configuration could not be established. "));
    }
    catch (uno::Exception & exception)
    {
        handleGeneralException(xProvider, exception,
                getMsgString( STR_CONFIG_ERR_ACCESS_GENERAL,
                            "A general error occurred while accessing your central configuration."));
    }
    return xProvider ;
}
// ----------------------------------------------------------------------------

static uno::Sequence< OUString > locateUserData(uno::Reference< lang::XMultiServiceFactory >  const & xLocatorFactory, OUString const & sUserDataSource )
{
    uno::Sequence< OUString > aResult;

    uno::Reference< task::XJob > xLocator( xLocatorFactory->createInstance( OUSTRING(USERDATA_LOCATOR) ), uno::UNO_QUERY);

    if (xLocator.is())
    {
        uno::Sequence< beans::NamedValue > aArgs(2);

        aArgs[0].Name  = OUSTRING("LayerDataUrl");
        aArgs[0].Value <<= sUserDataSource;

        OUString aUserProfile = OUSTRING("org.openoffice.UserProfile");
        uno::Sequence< OUString > aSkipComponents(&aUserProfile,1);
        aArgs[1].Name  = OUSTRING("ExcludeComponents");
        aArgs[1].Value <<= aSkipComponents;

        uno::Any aFound = xLocator->execute(aArgs);
        aFound >>= aResult;
    }
    else
        OSL_TRACE("Configuration - Import of user settings into new backend failed: No Locator Service available\n");

    return aResult;
}
// ----------------------------------------------------------------------------

static void copyUserData(uno::Reference< lang::XMultiServiceFactory >  const & xImporterFactory, uno::Sequence< OUString > const & sUserDataLayers )
{
    uno::Reference< task::XJob > xImporter( xImporterFactory->createInstance(OUSTRING(USERDATA_IMPORTER)), uno::UNO_QUERY);

    if (xImporter.is())
    {
        uno::Sequence< beans::NamedValue > aArgs(3);
        aArgs[0].Name  = OUSTRING("LayerDataUrl");
        aArgs[1].Name  = OUSTRING("OverwriteExisting");
        aArgs[1].Value <<= sal_False;
        aArgs[2].Name  = OUSTRING("ImporterService");
        aArgs[2].Value <<= OUSTRING(USERDATA_IMPORTSERVICE);

        for (sal_Int32 i=0; i < sUserDataLayers.getLength(); ++i)
        {
            aArgs[0].Value <<= sUserDataLayers[i];

            xImporter->execute(aArgs);
        }
        // TODO: If org.openoffice.Setup was copied, refresh data in cache
    }
    else
        OSL_TRACE("Configuration - Import of user settings into new backend failed: No Importer Service available\n");
}
// ----------------------------------------------------------------------------

static void maybeCopyUserData( ConfigurationProvider const & xProvider )
{
    OUString aUserDataSourceURL;

    // TODO: use "Context" property of xProvider to retrieve the information
    {
        static char const CONFIGRC[] = "$SYSBINDIR/" SAL_CONFIGFILE("configmgr");
        OUString sConfigIni = OUSTRING( CONFIGRC );
        rtl::Bootstrap::expandMacros(sConfigIni);

        rtl::Bootstrap aConfigInfo(sConfigIni);

        if (!aConfigInfo.getFrom( OUSTRING(INITUSERDATA_ENTRY), aUserDataSourceURL ) )
            return;
    }

    if (aUserDataSourceURL.getLength() == 0)
        return;

    uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();

    uno::Sequence< OUString > aDataUrls = locateUserData( xMSF, aUserDataSourceURL);
    if (aDataUrls.getLength() == 0)
        return;

    QueryBox aAskUser( NULL, ResId( QBX_CONFIG_IMPORTSETTINGS, Desktop::GetDesktopResManager() ) );
    setMsgBoxTitle(aAskUser);

    if (aAskUser.Execute() == RET_YES)
        copyUserData(xMSF, aDataUrls);
}
// ----------------------------------------------------------------------------

static void initializeUserData( ConfigurationProvider const & xProvider )
{
    OSL_ASSERT( xProvider.is() );

    try
    {
        uno::Reference< beans::XPropertySet > xSettings( getConfigurationSettings(xProvider,true), uno::UNO_QUERY );
        if (xSettings.is())
        {
            OUString const aSetting = OUSTRING(SETTING_DOIMPORT);

            sal_Bool bDoImport = false;
            xSettings->getPropertyValue( aSetting ) >>= bDoImport;

            if ( bDoImport )
            {
                maybeCopyUserData( xProvider );
                xSettings->setPropertyValue( aSetting, uno::makeAny(sal_False) );

                uno::Reference< util::XChangesBatch > xCommitSettings(xSettings, uno::UNO_QUERY);
                OSL_ENSURE(xCommitSettings.is(), "Missing interface to commit configuration change\n");
                if (xCommitSettings.is()) xCommitSettings->commitChanges();
            }
        }
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE( "Configuration - Import of user settings into new backend failed: %s\n",
                    OU2O(e.Message,ASCII_US).getStr() );
    }

}
// ----------------------------------------------------------------------------

static uno::Reference< uno::XInterface > getConfigurationSettings( ConfigurationProvider const & xProvider, bool bUpdate )
{
    if ( xProvider.is() )
    try
    {
        OUString sService = bUpdate ? OUSTRING(UPDATE_ACCESS) : OUSTRING(READONLY_ACCESS);

        OUString const sNodepath = OUSTRING(CONFIGURATION_SETTINGS);

        uno::Sequence< uno::Any > aArguments(1);
        aArguments[0] <<= beans::NamedValue( OUSTRING("nodepath"), uno::makeAny(sNodepath) );

        return xProvider->createInstanceWithArguments(sService,aArguments);
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE( "Configuration - Cannot get settings for configuration service: %s\n",
                    OU2O(e.Message,ASCII_US).getStr() );

    }
    return uno::Reference< uno::XInterface >();
}
// ----------------------------------------------------------------------------

