/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatecheck.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-01 10:12:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

// #ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
// #include <com/sun/star/beans/XPropertyState.hpp>
// #endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTEVENT_HPP_
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif

#include <comphelper/processfactory.hxx>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#include <rtl/bootstrap.hxx>
#include <osl/thread.h>
#include <osl/conditn.h>
#include <osl/process.h>
#include <osl/module.h>

#include "updateprotocol.hxx"
#include "updatecheckconfig.hxx"

namespace awt = com::sun::star::awt ;
namespace beans = com::sun::star::beans ;
namespace container = com::sun::star::container ;
namespace frame = com::sun::star::frame ;
namespace lang = com::sun::star::lang ;
namespace c3s = com::sun::star::system ;
namespace task = com::sun::star::task ;
namespace util = com::sun::star::util ;
namespace uno = com::sun::star::uno ;

extern "C" void SAL_CALL myThreadFunc(void*);

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

#define PROPERTY_TITLE          UNISTRING("BubbleHeading")
#define PROPERTY_TEXT           UNISTRING("BubbleText")
#define PROPERTY_IMAGE          UNISTRING("BubbleImageURL")
#define PROPERTY_SHOW_BUBBLE    UNISTRING("BubbleVisible")
#define PROPERTY_CLICK_HDL      UNISTRING("MenuClickHDL")
#define PROPERTY_DEFAULT_TITLE  UNISTRING("DefaultHeading")
#define PROPERTY_DEFAULT_TEXT   UNISTRING("DefaultText")
#define PROPERTY_SHOW_MENUICON  UNISTRING("MenuIconVisible")

//------------------------------------------------------------------------------

template <typename T>
T getValue( const uno::Sequence< beans::NamedValue >& rNamedValues, const sal_Char * pszName )
    throw (uno::RuntimeException)
{
    for( sal_Int32 n=0; n < rNamedValues.getLength(); n++ )
    {
    // Unfortunatly gcc-3.3 does not like Any.get<T>();
        if( rNamedValues[n].Name.equalsAscii( pszName ) )
        {
            T value;
            if( ! (rNamedValues[n].Value >>= value) )
                throw uno::RuntimeException(
                    ::rtl::OUString(
                        cppu_Any_extraction_failure_msg(
                            &rNamedValues[n].Value,
                            ::cppu::getTypeFavourUnsigned(&value).getTypeLibType() ),
                            SAL_NO_ACQUIRE ),
                    uno::Reference<uno::XInterface>() );

            return value;
        }
    }

    return T();
}

//------------------------------------------------------------------------------

template <typename T>
void setValue( uno::Sequence< beans::NamedValue >& rNamedValues, const sal_Char * pszName, T const & value )
    throw (uno::RuntimeException)
{
    for( sal_Int32 n=0; n < rNamedValues.getLength(); n++ )
        if( rNamedValues[n].Name.equalsAscii( pszName ) )
            rNamedValues[n].Value = uno::makeAny( value );
}

//------------------------------------------------------------------------------

namespace
{

class UpdateCheckJob : public ::cppu::WeakImplHelper3< task::XJob, lang::XServiceInfo, util::XChangesListener >
{
    uno::Reference<uno::XComponentContext> m_xContext;

    uno::Reference<lang::XMultiServiceFactory> getConfigProvider() const;

    uno::Reference< uno::XInterface > getConfigAccess(
        rtl::OUString const & rNodePath,
        rtl::OUString const & rService,
        uno::Reference< lang::XMultiServiceFactory > const & rConfigProvider ) const;

    uno::Reference< container::XNameAccess > getOwnConfigAccess() const;

    inline uno::Reference< container::XNameAccess > getNameAccess(
        rtl::OUString const & rNodePath,
        uno::Reference< lang::XMultiServiceFactory > const & rConfigProvider ) const;

    inline uno::Reference< container::XNameReplace > getUpdateAccess(
        rtl::OUString const & rNodePath,
        uno::Reference< lang::XMultiServiceFactory > const & rConfigProvider ) const;

    inline uno::Reference< container::XNameAccess > getNameAccess(rtl::OUString const & rNodePath) const
        { return getNameAccess(rNodePath, getConfigProvider()); };
    inline uno::Reference< container::XNameReplace > getUpdateAccess(rtl::OUString const & rNodePath) const
        { return getUpdateAccess(rNodePath, getConfigProvider()); };



    uno::Reference<uno::XInterface> getUIService(sal_Bool bShowBubble) const;
    uno::Reference<task::XInteractionHandler> getInteractionHandler() const;
    uno::Reference<c3s::XSystemShellExecute> getShellExecuter() const;

    static uno::Reference< uno::XInterface > g_aInstance;

    // Not using the C++ wrappers because we need those in the global instance only
    oslThread m_hThread;
    oslModule m_hModule;
    oslCondition m_hCondition;

    void makeGlobal(const uno::Sequence<beans::NamedValue>& rNamedValues)
        throw (uno::RuntimeException);
    void createAndStartThread( rtl::OUString &rBuildID )
        throw (uno::RuntimeException);

    rtl::OUString m_aBuildId;
    rtl::OUString getBuildId();

    uno::Reference< uno::XInterface > m_xUIService;

    sal_Bool (* m_pHasInternetConnection) ();

protected:
    void runAsThread();
    friend void SAL_CALL ::myThreadFunc(void *);

    inline rtl::OUString getProductName() const;
    inline rtl::OUString getLocale() const;

    /* Used to avoid dialup login windows (on platforms we know how to double this) */
    inline bool hasInternetConnection() const
    {
        if(m_pHasInternetConnection != NULL )
            return (sal_True == m_pHasInternetConnection());
        return true;
    }

public:
    UpdateCheckJob(const uno::Reference<uno::XComponentContext>&);
    virtual ~UpdateCheckJob();

    static uno::Sequence< rtl::OUString > getServiceNames();
    static rtl::OUString getImplName();

    // Allows runtime exceptions to be thrown by const methods
    inline SAL_CALL operator uno::Reference< uno::XInterface > () const
        { return const_cast< cppu::OWeakObject * > (static_cast< cppu::OWeakObject const * > (this)); };

    // XJob
    virtual uno::Any SAL_CALL execute(const uno::Sequence<beans::NamedValue>&)
        throw (lang::IllegalArgumentException, uno::Exception);

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const util::ChangesEvent& Event )
        throw ( uno::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (uno::RuntimeException);
    virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);

    virtual void SAL_CALL disposing(const lang::EventObject&)
        throw (uno::RuntimeException);
};

uno::Reference< uno::XInterface > UpdateCheckJob::g_aInstance;

//------------------------------------------------------------------------------

UpdateCheckJob::UpdateCheckJob(const uno::Reference<uno::XComponentContext>& xContext) :
    m_xContext(xContext),
    m_pHasInternetConnection(NULL)
{
    m_hThread = 0;
    m_hModule = 0;
    m_hCondition = 0;
}

//------------------------------------------------------------------------------

UpdateCheckJob::~UpdateCheckJob()
{
    if( m_hThread )
    {
        osl_terminateThread(m_hThread);
        osl_setCondition(m_hCondition);

        osl_joinWithThread(m_hThread);
        osl_destroyThread(m_hThread);
    }

    if( m_hCondition )
        osl_destroyCondition(m_hCondition);

    if( m_hModule )
        osl_unloadModule(m_hModule);
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString >
UpdateCheckJob::getServiceNames()
{
    uno::Sequence< rtl::OUString > aServiceList(1);
    aServiceList[0] = UNISTRING( "com.sun.star.setup.UpdateCheck");
    return aServiceList;
};

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckJob::getImplName()
{
    return UNISTRING( "vnd.sun.UpdateCheck");
}

//------------------------------------------------------------------------------

uno::Reference< lang::XMultiServiceFactory >
UpdateCheckJob::getConfigProvider() const
{
    uno::Reference<uno::XComponentContext> xContext(m_xContext);

    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: empty component context" ), *this );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: unable to obtain service manager from component context" ), *this );

    uno::Reference< lang::XMultiServiceFactory > xConfigurationProvider(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.configuration.ConfigurationProvider" ), xContext ),
        uno::UNO_QUERY_THROW);

    return xConfigurationProvider;
}

//------------------------------------------------------------------------------

uno::Reference< uno::XInterface >
UpdateCheckJob::getConfigAccess(
    const rtl::OUString& rNodePath,
    const rtl::OUString& rService,
    uno::Reference< lang::XMultiServiceFactory > const & rxConfigProvider) const
{
    beans::PropertyValue aProperty;
    aProperty.Name  = UNISTRING( "nodepath" );
    aProperty.Value = uno::makeAny( rNodePath );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    return rxConfigProvider->createInstanceWithArguments( rService, aArgumentList );
}

//------------------------------------------------------------------------------
uno::Reference< container::XNameAccess >
UpdateCheckJob::getOwnConfigAccess() const
{
    uno::Reference<uno::XComponentContext> xContext(m_xContext);

    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: empty component context" ), *this );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: unable to obtain service manager from component context" ), *this );

    uno::Reference< container::XNameAccess > xNameAccess(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.setup.UpdateCheckConfig" ), xContext ),
        uno::UNO_QUERY_THROW);

    return xNameAccess;
}

//------------------------------------------------------------------------------

inline uno::Reference< container::XNameAccess >
UpdateCheckJob::getNameAccess(
    rtl::OUString const & rNodePath,
    uno::Reference< lang::XMultiServiceFactory > const & rxConfigProvider) const
{
    return uno::Reference< container::XNameAccess > (
        getConfigAccess( rNodePath,
            UNISTRING("com.sun.star.configuration.ConfigurationAccess"),
            rxConfigProvider ),
        uno::UNO_QUERY_THROW );
}

//------------------------------------------------------------------------------

inline uno::Reference< container::XNameReplace >
UpdateCheckJob::getUpdateAccess(
    rtl::OUString const & rNodePath,
    uno::Reference< lang::XMultiServiceFactory > const & rxConfigProvider) const
{
    return uno::Reference< container::XNameReplace > (
        getConfigAccess( rNodePath,
            UNISTRING("com.sun.star.configuration.ConfigurationUpdateAccess"),
            rxConfigProvider ),
        uno::UNO_QUERY_THROW );
}

//------------------------------------------------------------------------------

uno::Reference< uno::XInterface >
UpdateCheckJob::getUIService(sal_Bool bShowBubble) const
{
    uno::Reference< uno::XComponentContext > xContext(m_xContext);
    uno::Reference< uno::XInterface > xUpdateCheckUI;

    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: empty component context" ), *this );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: unable to obtain service manager from component context" ), *this );

    xUpdateCheckUI = xServiceManager->createInstanceWithContext(
        UNISTRING( "com.sun.star.setup.UpdateCheckUI" ), xContext );

    uno::Reference< beans::XPropertySet > xSetProperties(xUpdateCheckUI, uno::UNO_QUERY_THROW);
    xSetProperties->setPropertyValue( PROPERTY_TITLE,
                                      xSetProperties->getPropertyValue( PROPERTY_DEFAULT_TITLE ) );
    xSetProperties->setPropertyValue( PROPERTY_TEXT,
                                      xSetProperties->getPropertyValue( PROPERTY_DEFAULT_TEXT ) );
    xSetProperties->setPropertyValue( PROPERTY_CLICK_HDL,
                                      uno::makeAny( uno::Reference< task::XJob >(const_cast <UpdateCheckJob *> (this) ) ) );
    xSetProperties->setPropertyValue( PROPERTY_SHOW_BUBBLE, uno::makeAny( bShowBubble ) );

    return xUpdateCheckUI;
}

//------------------------------------------------------------------------------

uno::Reference<task::XInteractionHandler>
UpdateCheckJob::getInteractionHandler() const
{
    uno::Reference<uno::XComponentContext> xContext(m_xContext);

    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: empty component context" ), *this );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: unable to obtain service manager from component context" ), *this );

    return uno::Reference<task::XInteractionHandler> (
        xServiceManager->createInstanceWithContext(
            UNISTRING( "com.sun.star.task.InteractionHandler" ),
            xContext),
        uno::UNO_QUERY_THROW);
}

//------------------------------------------------------------------------------

uno::Reference<c3s::XSystemShellExecute>
UpdateCheckJob::getShellExecuter() const
{
    uno::Reference<uno::XComponentContext> xContext(m_xContext);

    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: empty component context" ), *this );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: unable to obtain service manager from component context" ), *this );

    return uno::Reference<c3s::XSystemShellExecute> (
        xServiceManager->createInstanceWithContext(
            UNISTRING( "com.sun.star.system.SystemShellExecute" ),
            xContext),
        uno::UNO_QUERY_THROW);
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckJob::getBuildId()
{
    if( m_aBuildId.getLength() == 0 )
    {
        rtl::OUString aPath;
        if( osl_getExecutableFile(&aPath.pData) == osl_Process_E_None )
        {
            sal_uInt32 lastIndex = aPath.lastIndexOf('/');
            if ( lastIndex > 0 )
            {
                aPath = aPath.copy( 0, lastIndex+1 );
                aPath  += UNISTRING( SAL_CONFIGFILE( "version" ) );
            }

            rtl::Bootstrap aVersionFile(aPath);
            aVersionFile.getFrom(UNISTRING("buildid"), m_aBuildId, rtl::OUString());
        }
    }

    return m_aBuildId;
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckJob::getProductName() const
{
    rtl::OUString aProductName;

    uno::Reference< container::XNameAccess > xNameAccess( getNameAccess(UNISTRING("org.openoffice.Setup/Product")) );
    xNameAccess->getByName(UNISTRING("ooName")) >>= aProductName;

    return aProductName;
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckJob::getLocale() const
{
    rtl::OUString aProductName;

    uno::Reference< container::XNameAccess > xNameAccess( getNameAccess(UNISTRING("org.openoffice.Setup/L10N")) );
    xNameAccess->getByName(UNISTRING("ooLocale")) >>= aProductName;

    return aProductName;
}

//------------------------------------------------------------------------------

void
UpdateCheckJob::runAsThread()
{
    TimeValue tv = { 10, 0 };

    // Initial wait to avoid doing further time consuming tasks during start-up
    osl_waitCondition(m_hCondition, &tv);

    // Might have been terminated meanwhile
    if( sal_False == osl_scheduleThread(m_hThread) )
        return;

    try {
        uno::Reference< container::XNameReplace > xCFGUpdate =
            getUpdateAccess( UNISTRING("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments") );

        while( sal_True == osl_scheduleThread(m_hThread) )
        {
            sal_Int64 last;
            sal_Int64 offset;
            sal_Bool bEnabled;

            xCFGUpdate->getByName( UNISTRING("LastCheck") ) >>= last;
            xCFGUpdate->getByName( UNISTRING("CheckInterval") ) >>= offset;
            xCFGUpdate->getByName( UNISTRING("AutoCheckEnabled") ) >>= bEnabled;

            if( bEnabled != sal_True )
            {
                osl_terminateThread(m_hThread);
                continue;
            }

            // last == 0 means check immediately
            if( last > 0 )
            {
                TimeValue systime;
                osl_getSystemTime(&systime);

                // Go back to sleep until time has elapsed
                sal_Int64 next = last + offset;
                if( last + offset > systime.Seconds )
                {
                    // This can not be > 32 Bit for now ..
                    tv.Seconds = static_cast< sal_Int32 > (next - systime.Seconds);
                    osl_waitCondition(m_hCondition, &tv);
                    continue;
                }
            }

            rtl::OUString aDownloadURL;
            rtl::OUString aVersionFound;

            if( hasInternetConnection() &&
                checkForUpdates(m_xContext, uno::Reference< task::XInteractionHandler >(), aDownloadURL, aVersionFound) )
            {
                /*
                 * found updates for previous version are removed at startup, so
                 * we just compare the version info returned from the server
                 */

                if( aDownloadURL.getLength() > 0 )
                {
                    rtl::OUString aPreviousVersionFound;
                    xCFGUpdate->getByName( UNISTRING("UpdateVersionFound") ) >>= aPreviousVersionFound;

                    if( ! aPreviousVersionFound.equals(aVersionFound) )
                        m_xUIService = getUIService(sal_True);

                    xCFGUpdate->replaceByName(UNISTRING("UpdateVersionFound"), uno::makeAny(aVersionFound));
                    xCFGUpdate->replaceByName(UNISTRING("DownloadURL"), uno::makeAny(aDownloadURL));
                    xCFGUpdate->replaceByName(UNISTRING("UpdateFoundFor"), uno::makeAny(getBuildId()));
                }

                TimeValue systime;
                osl_getSystemTime(&systime);
                sal_Int64 lastCheck = systime.Seconds;

                xCFGUpdate->replaceByName(UNISTRING("LastCheck"), uno::makeAny(lastCheck));

                uno::Reference< util::XChangesBatch > xChangesBatch(xCFGUpdate, uno::UNO_QUERY_THROW);
                xChangesBatch->commitChanges();
            }
            else
            {
                // Increase next by 1, 5, 15, 60, .. minutes
                static const sal_Int16 nRetryInterval[] = { 60, 300, 900, 3600, 0 /* Ignored */ };
                static sal_uInt8 n = 0;

                if( n < sizeof(nRetryInterval) / sizeof(sal_Int16) )
                    ++n;

                tv.Seconds = nRetryInterval[n];
                osl_waitCondition(m_hCondition, &tv);
                continue;
            }
        }
    }

    catch(const uno::Exception& e) {
        // Silently catch all errors
        OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr() );
    }

    m_hThread = 0;
}

//------------------------------------------------------------------------------

void
UpdateCheckJob::makeGlobal(const uno::Sequence<beans::NamedValue>& rNamedValues)
    throw (uno::RuntimeException)
{
    rtl::OUString aBuildId = getValue< rtl::OUString > (rNamedValues, "UpdateFoundFor");

    // Clean up service data after update
    if( aBuildId.getLength() > 0 && ! aBuildId.equals(getBuildId()) )
    {
        uno::Reference< container::XNameReplace > xCFGUpdate =
            getUpdateAccess(UNISTRING("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments"));

        uno::Any aEmptyAny = uno::makeAny( rtl::OUString() );

        xCFGUpdate->replaceByName(UNISTRING("UpdateVersionFound"), aEmptyAny);
        xCFGUpdate->replaceByName(UNISTRING("DownloadURL"), aEmptyAny);
        xCFGUpdate->replaceByName(UNISTRING("UpdateFoundFor"), aEmptyAny);

        uno::Reference< util::XChangesBatch > xChangesBatch(xCFGUpdate, uno::UNO_QUERY_THROW);
        xChangesBatch->commitChanges();

        aBuildId = rtl::OUString();
    }

    sal_Bool isEnabled = getValue< sal_Bool > (rNamedValues, "AutoCheckEnabled");
    if( isEnabled == sal_True )
    {
        createAndStartThread( aBuildId );
    }
}

//------------------------------------------------------------------------------

void
UpdateCheckJob::createAndStartThread( rtl::OUString &rBuildId )
    throw (uno::RuntimeException)
{
    // If we found an update earlier, turn on menu bar icon
    if( ( rBuildId.getLength() > 0 ) && ! m_xUIService.is() )
    {
        m_xUIService = getUIService(sal_False);
        OSL_TRACE( "UI Service initialization %s\n", m_xUIService.is() ? "succeeded" : "failed" );
    }

    // Initialize thread resources ..
    if ( m_hCondition == 0 )
        m_hCondition = osl_createCondition();
    if( m_hCondition == 0 )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: unable to create condition object" ), *this);

    if ( m_hThread == 0 )
        m_hThread = osl_createSuspendedThread( myThreadFunc, this );
    if( m_hThread == 0 )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckJob: unable to create thread object" ), *this);

    // .. , hook up as terminate listener and remember this instace before ..
    if ( ! g_aInstance.is() )
    {
        uno::Reference< lang::XComponent > xComponent(m_xContext, uno::UNO_QUERY_THROW);

        xComponent->addEventListener(this);
        g_aInstance = static_cast< cppu::OWeakObject *> (this);
    }

#ifdef WNT
    rtl::OUString aPath;
    if( osl_getExecutableFile(&aPath.pData) == osl_Process_E_None )
    {
        sal_uInt32 lastIndex = aPath.lastIndexOf('/');
        if ( lastIndex > 0 )
        {
            aPath = aPath.copy( 0, lastIndex+1 );
            aPath  += UNISTRING( "onlinecheck" );
        }

        if ( ! m_hModule )
        {
            m_hModule = osl_loadModule(aPath.pData, SAL_LOADMODULE_DEFAULT);

            if( m_hModule )
                m_pHasInternetConnection = reinterpret_cast < sal_Bool (*) () > (
                    osl_getFunctionSymbol(m_hModule,
                        UNISTRING("hasInternetConnection").pData));
        }
    }
#endif

    // actually run the thread
    osl_resumeThread(m_hThread);
}

//------------------------------------------------------------------------------

uno::Any
UpdateCheckJob::execute(const uno::Sequence<beans::NamedValue>& namedValues)
    throw (lang::IllegalArgumentException, uno::Exception)
{
    uno::Sequence<beans::NamedValue> aConfig =
        getValue< uno::Sequence<beans::NamedValue> > (namedValues, "JobConfig");

    /* Determine the way we got invoked here -
     * see Developers Guide Chapter "4.7.2 Jobs" to understand the magic
     */

    uno::Sequence<beans::NamedValue> aEnvironment =
        getValue< uno::Sequence<beans::NamedValue> > (namedValues, "Environment");

    rtl::OUString aEventName = getValue< rtl::OUString > (aEnvironment, "EventName");

    if( aEventName.equalsAscii("onFirstVisibleTask") )
    {
        makeGlobal(aConfig);

        uno::Reference < util::XChangesNotifier > xChangesNotifier;
        xChangesNotifier = uno::Reference < util::XChangesNotifier >( getOwnConfigAccess(), uno::UNO_QUERY_THROW );

        if ( xChangesNotifier.is() )
            xChangesNotifier->addChangesListener( uno::Reference< util::XChangesListener >(const_cast <UpdateCheckJob *> (this) ) );
    }
    else if( aConfig.getLength() > 0 )
    {
        uno::Reference < container::XNameReplace > xCFGUpdate;
        xCFGUpdate = uno::Reference < container::XNameReplace >( getOwnConfigAccess(), uno::UNO_QUERY_THROW );

        rtl::OUString aDownloadURL;
        rtl::OUString aVersionFound;
        uno::Sequence< beans::NamedValue > aResult(1);
        aResult[0].Name = UNISTRING("SendDispatchResult");

        if( checkForUpdates(m_xContext, getInteractionHandler(), aDownloadURL, aVersionFound) )
        {
            if( aDownloadURL.getLength() > 0 )
            {
                xCFGUpdate->replaceByName(UNISTRING("UpdateVersionFound"), uno::makeAny(aVersionFound));
                xCFGUpdate->replaceByName(UNISTRING("DownloadURL"), uno::makeAny(aDownloadURL));
                xCFGUpdate->replaceByName(UNISTRING("UpdateFoundFor"), uno::makeAny(getBuildId()));
            }
            else
            {
                uno::Any aEmptyAny = uno::makeAny( rtl::OUString() );
                xCFGUpdate->replaceByName(UNISTRING("UpdateFoundFor"), aEmptyAny);
            }

            TimeValue systime;
            osl_getSystemTime(&systime);
            sal_Int64 lastCheck = systime.Seconds;

            xCFGUpdate->replaceByName(UNISTRING("LastCheck"), uno::makeAny(lastCheck));

            uno::Reference< util::XChangesBatch > xChangesBatch(xCFGUpdate, uno::UNO_QUERY_THROW);
            xChangesBatch->commitChanges();

            // setValue< sal_Int64 > (aConfig, "NextCheck", nOffset + systime.Seconds);

            frame::DispatchResultEvent aResultEvent(*this, frame::DispatchResultState::SUCCESS, uno::makeAny(aDownloadURL));

            aResult[0].Value = uno::makeAny(aResultEvent);
        }
        else
        {
            aResult[0].Value = uno::makeAny(sal_False);
        }

        return uno::makeAny(aResult);
    }
    else
    {
        uno::Reference< container::XNameAccess > xCFGAccess =
            getNameAccess( UNISTRING("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments") );

        rtl::OUString aDownloadURL;
        xCFGAccess->getByName( UNISTRING("DownloadURL") ) >>= aDownloadURL;

        uno::Reference< c3s::XSystemShellExecute > xShellExecute( getShellExecuter() );
        xShellExecute->execute( aDownloadURL, ::rtl::OUString(), c3s::SystemShellExecuteFlags::DEFAULTS );
    }

    return uno::Any();
}

//------------------------------------------------------------------------------
// XChangesListener

void SAL_CALL
UpdateCheckJob::changesOccurred( const util::ChangesEvent& rEvent )
        throw ( uno::RuntimeException )
{
    sal_Int32 nCount = rEvent.Changes.getLength();
    rtl::OUString aString;

    for ( sal_Int32 i=0; i<nCount; i++ )
    {
        uno::Any aAccessor = rEvent.Changes[i].Accessor;
        aAccessor >>= aString;
        if ( aString.indexOf( UNISTRING( "AutoCheckEnabled" ) ) != -1 )
        {
            sal_Bool bAutoCheck;
            rEvent.Changes[i].Element >>= bAutoCheck;

            if ( m_xUIService.is() )
            {
                uno::Reference< beans::XPropertySet > xSetProperties(m_xUIService, uno::UNO_QUERY_THROW);
                xSetProperties->setPropertyValue( PROPERTY_SHOW_MENUICON, uno::makeAny( bAutoCheck ) );
            }
            if ( bAutoCheck )
            {
                uno::Reference< container::XNameReplace > xCFGUpdate =
                    getUpdateAccess( UNISTRING("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments") );
                xCFGUpdate->getByName( UNISTRING("UpdateFoundFor") ) >>= aString;

                createAndStartThread( aString );
            }
            else if ( m_hThread )
            {
                osl_terminateThread(m_hThread);
                osl_setCondition(m_hCondition);
                osl_joinWithThread(m_hThread);
                osl_destroyThread(m_hThread);
                osl_resetCondition(m_hCondition);
                m_hThread = 0;
            }
        }
        else if ( aString.indexOf( UNISTRING( "CheckInterval" ) ) != -1 )
        {
            if ( m_hThread )
            {
                osl_setCondition(m_hCondition);
                osl_resetCondition(m_hCondition);
            }
        }
        else if ( aString.indexOf( UNISTRING( "UpdateFoundFor" ) ) != -1 )
        {
            rEvent.Changes[i].Element >>= aString;

            sal_Bool bAutoCheck = sal_False;
            uno::Reference< container::XNameReplace > xCFGUpdate =
                getUpdateAccess( UNISTRING("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments") );
            xCFGUpdate->getByName( UNISTRING("AutoCheckEnabled") ) >>= bAutoCheck;

            if ( m_xUIService.is() )
            {
                uno::Reference< beans::XPropertySet > xSetProperties(m_xUIService, uno::UNO_QUERY_THROW);

                if ( ( aString.getLength() == 0 ) && xSetProperties.is() )
                    xSetProperties->setPropertyValue( PROPERTY_SHOW_MENUICON, uno::makeAny( sal_False ) );
                else if ( ( aString.getLength() != 0 )&& xSetProperties.is() && bAutoCheck )
                    xSetProperties->setPropertyValue( PROPERTY_SHOW_MENUICON, uno::makeAny( sal_True ) );
            }
            else if ( ( aString.getLength() != 0 ) && bAutoCheck )
                createAndStartThread( aString );
        }
    }
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
UpdateCheckJob::getImplementationName() throw (uno::RuntimeException)
{
    return getImplName();
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL
UpdateCheckJob::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getServiceNames();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckJob::supportsService( rtl::OUString const & serviceName ) throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aServiceNameList = getServiceNames();

    for( sal_Int32 n=0; n < aServiceNameList.getLength(); n++ )
        if( aServiceNameList[n].equals(serviceName) )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------

void SAL_CALL
UpdateCheckJob::disposing(const lang::EventObject&) throw (uno::RuntimeException)
{
    g_aInstance.clear();
}

} // anonymous namespace

//------------------------------------------------------------------------------

extern "C" void SAL_CALL myThreadFunc( void* p )
{
    reinterpret_cast< UpdateCheckJob * > (p)->runAsThread();
}

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL
createJobInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    return * new UpdateCheckJob(xContext);
}

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL
createConfigInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    static uno::Reference<uno::XInterface> xConfig;

    if ( !xConfig.is() )
        xConfig = * new UpdateCheckConfig( xContext );
    return xConfig;
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createJobInstance,
        UpdateCheckJob::getImplName,
        UpdateCheckJob::getServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    {
        createConfigInstance,
        UpdateCheckConfig::getImplName,
        UpdateCheckConfig::getServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;

//------------------------------------------------------------------------------

extern "C" void SAL_CALL
component_getImplementationEnvironment( const sal_Char **aEnvTypeName, uno_Environment **)
{
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL
component_writeInfo(void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_writeInfoHelper(
        pServiceManager,
        pRegistryKey,
        kImplementations_entries
    );
}

//------------------------------------------------------------------------------

extern "C" void *
component_getFactory(const sal_Char *pszImplementationName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pszImplementationName,
        pServiceManager,
        pRegistryKey,
        kImplementations_entries) ;
}

