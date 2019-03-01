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

#include "datasource.hxx"
#include "commandcontainer.hxx"
#include <stringconstants.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include "connection.hxx"
#include "SharedConnection.hxx"
#include "databasedocument.hxx"
#include <OAuthenticationContinuation.hxx>

#include <hsqlimport.hxx>
#include <migrwarndlg.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdbc/ConnectionPool.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbc/XDriverManager.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/view/XPrintable.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>
#include <typelib/typedescription.hxx>
#include <unotools/confignode.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <rtl/digest.h>

#include <algorithm>
#include <iterator>
#include <set>

#include <config_firebird.h>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::reflection;
using namespace ::cppu;
using namespace ::osl;
using namespace ::dbtools;
using namespace ::comphelper;

namespace dbaccess
{

/** helper class which implements a XFlushListener, and forwards all
    notification events to another XFlushListener

    The speciality is that the foreign XFlushListener instance, to which
    the notifications are forwarded, is held weak.

    Thus, the class can be used with XFlushable instance which hold
    their listeners with a hard reference, if you simply do not *want*
    to be held hard-ref-wise.
*/
class FlushNotificationAdapter : public ::cppu::WeakImplHelper< XFlushListener >
{
private:
    WeakReference< XFlushable >     m_aBroadcaster;
    WeakReference< XFlushListener > m_aListener;

public:
    static void installAdapter( const Reference< XFlushable >& _rxBroadcaster, const Reference< XFlushListener >& _rxListener )
    {
        Reference< XFlushListener > xAdapter( new FlushNotificationAdapter( _rxBroadcaster, _rxListener ) );
    }

protected:
    FlushNotificationAdapter( const Reference< XFlushable >& _rxBroadcaster, const Reference< XFlushListener >& _rxListener );
    virtual ~FlushNotificationAdapter() override;

    void impl_dispose();

protected:
    // XFlushListener
    virtual void SAL_CALL flushed( const css::lang::EventObject& rEvent ) override;
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
};

FlushNotificationAdapter::FlushNotificationAdapter( const Reference< XFlushable >& _rxBroadcaster, const Reference< XFlushListener >& _rxListener )
    :m_aBroadcaster( _rxBroadcaster )
    ,m_aListener( _rxListener )
{
    OSL_ENSURE( _rxBroadcaster.is(), "FlushNotificationAdapter::FlushNotificationAdapter: invalid flushable!" );

    osl_atomic_increment( &m_refCount );
    {
        if ( _rxBroadcaster.is() )
            _rxBroadcaster->addFlushListener( this );
    }
    osl_atomic_decrement( &m_refCount );
    OSL_ENSURE( m_refCount == 1, "FlushNotificationAdapter::FlushNotificationAdapter: broadcaster isn't holding by hard ref!?" );
}

FlushNotificationAdapter::~FlushNotificationAdapter()
{
}

void FlushNotificationAdapter::impl_dispose()
{
    Reference< XFlushListener > xKeepAlive( this );

    Reference< XFlushable > xFlushable( m_aBroadcaster );
    if ( xFlushable.is() )
        xFlushable->removeFlushListener( this );

    m_aListener.clear();
    m_aBroadcaster.clear();
}

void SAL_CALL FlushNotificationAdapter::flushed( const EventObject& rEvent )
{
    Reference< XFlushListener > xListener( m_aListener );
    if ( xListener.is() )
        xListener->flushed( rEvent );
    else
        impl_dispose();
}

void SAL_CALL FlushNotificationAdapter::disposing( const EventObject& Source )
{
    Reference< XFlushListener > xListener( m_aListener );
    if ( xListener.is() )
        xListener->disposing( Source );

    impl_dispose();
}

OAuthenticationContinuation::OAuthenticationContinuation()
    :m_bRemberPassword(true),   // TODO: a meaningful default
    m_bCanSetUserName(true)
{
}

sal_Bool SAL_CALL OAuthenticationContinuation::canSetRealm(  )
{
    return false;
}

void SAL_CALL OAuthenticationContinuation::setRealm( const OUString& /*Realm*/ )
{
    SAL_WARN("dbaccess","OAuthenticationContinuation::setRealm: not supported!");
}

sal_Bool SAL_CALL OAuthenticationContinuation::canSetUserName(  )
{
    // we always allow this, even if the database document is read-only. In this case,
    // it's simply that the user cannot store the new user name.
    return m_bCanSetUserName;
}

void SAL_CALL OAuthenticationContinuation::setUserName( const OUString& _rUser )
{
    m_sUser = _rUser;
}

sal_Bool SAL_CALL OAuthenticationContinuation::canSetPassword(  )
{
    return true;
}

void SAL_CALL OAuthenticationContinuation::setPassword( const OUString& _rPassword )
{
    m_sPassword = _rPassword;
}

Sequence< RememberAuthentication > SAL_CALL OAuthenticationContinuation::getRememberPasswordModes( RememberAuthentication& _reDefault )
{
    Sequence< RememberAuthentication > aReturn(1);
    _reDefault = aReturn[0] = RememberAuthentication_SESSION;
    return aReturn;
}

void SAL_CALL OAuthenticationContinuation::setRememberPassword( RememberAuthentication _eRemember )
{
    m_bRemberPassword = (RememberAuthentication_NO != _eRemember);
}

sal_Bool SAL_CALL OAuthenticationContinuation::canSetAccount(  )
{
    return false;
}

void SAL_CALL OAuthenticationContinuation::setAccount( const OUString& )
{
    SAL_WARN("dbaccess","OAuthenticationContinuation::setAccount: not supported!");
}

Sequence< RememberAuthentication > SAL_CALL OAuthenticationContinuation::getRememberAccountModes( RememberAuthentication& _reDefault )
{
    Sequence < RememberAuthentication > aReturn(1);
    aReturn[0] = RememberAuthentication_NO;
    _reDefault = RememberAuthentication_NO;
    return aReturn;
}

void SAL_CALL OAuthenticationContinuation::setRememberAccount( RememberAuthentication /*Remember*/ )
{
    SAL_WARN("dbaccess","OAuthenticationContinuation::setRememberAccount: not supported!");
}

/** The class OSharedConnectionManager implements a structure to share connections.
    It owns the master connections which will be disposed when the last connection proxy is gone.
*/
// need to hold the digest
struct TDigestHolder
{
    sal_uInt8 m_pBuffer[RTL_DIGEST_LENGTH_SHA1];
    TDigestHolder()
    {
        m_pBuffer[0] = 0;
    }

};

class OSharedConnectionManager : public ::cppu::WeakImplHelper< XEventListener >
{

     // contains the currently used master connections
    struct TConnectionHolder
    {
        Reference< XConnection >    xMasterConnection;
        oslInterlockedCount         nALiveCount;
    };

    // the less-compare functor, used for the stl::map
    struct TDigestLess
    {
        bool operator() (const TDigestHolder& x, const TDigestHolder& y) const
        {
            sal_uInt32 i;
            for(i=0;i < RTL_DIGEST_LENGTH_SHA1 && (x.m_pBuffer[i] >= y.m_pBuffer[i]); ++i)
                ;
            return i < RTL_DIGEST_LENGTH_SHA1;
        }
    };

    typedef std::map< TDigestHolder,TConnectionHolder,TDigestLess>        TConnectionMap;      // holds the master connections
    typedef std::map< Reference< XConnection >,TConnectionMap::iterator>  TSharedConnectionMap;// holds the shared connections

    ::osl::Mutex                m_aMutex;
    TConnectionMap              m_aConnections;         // remember the master connection in conjunction with the digest
    TSharedConnectionMap        m_aSharedConnection;    // the shared connections with conjunction with an iterator into the connections map
    Reference< XProxyFactory >  m_xProxyFactory;

protected:
    virtual ~OSharedConnectionManager() override;

public:
    explicit OSharedConnectionManager(const Reference< XComponentContext >& _rxContext);

    void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    Reference<XConnection> getConnection(   const OUString& url,
                                            const OUString& user,
                                            const OUString& password,
                                            const Sequence< PropertyValue >& _aInfo,
                                            ODatabaseSource* _pDataSource);
    void addEventListener(const Reference<XConnection>& _rxConnection, TConnectionMap::iterator const & _rIter);
};

OSharedConnectionManager::OSharedConnectionManager(const Reference< XComponentContext >& _rxContext)
{
    m_xProxyFactory.set( ProxyFactory::create( _rxContext ) );
}

OSharedConnectionManager::~OSharedConnectionManager()
{
}

void SAL_CALL OSharedConnectionManager::disposing( const css::lang::EventObject& Source )
{
    MutexGuard aGuard(m_aMutex);
    Reference<XConnection> xConnection(Source.Source,UNO_QUERY);
    TSharedConnectionMap::const_iterator aFind = m_aSharedConnection.find(xConnection);
    if ( m_aSharedConnection.end() != aFind )
    {
        osl_atomic_decrement(&aFind->second->second.nALiveCount);
        if ( !aFind->second->second.nALiveCount )
        {
            ::comphelper::disposeComponent(aFind->second->second.xMasterConnection);
            m_aConnections.erase(aFind->second);
        }
        m_aSharedConnection.erase(aFind);
    }
}

Reference<XConnection> OSharedConnectionManager::getConnection( const OUString& url,
                                        const OUString& user,
                                        const OUString& password,
                                        const Sequence< PropertyValue >& _aInfo,
                                        ODatabaseSource* _pDataSource)
{
    MutexGuard aGuard(m_aMutex);
    TConnectionMap::key_type nId;
    Sequence< PropertyValue > aInfoCopy(_aInfo);
    sal_Int32 nPos = aInfoCopy.getLength();
    aInfoCopy.realloc( nPos + 2 );
    aInfoCopy[nPos].Name      = "TableFilter";
    aInfoCopy[nPos++].Value <<= _pDataSource->m_pImpl->m_aTableFilter;
    aInfoCopy[nPos].Name      = "TableTypeFilter";
    aInfoCopy[nPos++].Value <<= _pDataSource->m_pImpl->m_aTableTypeFilter;

    OUString sUser = user;
    OUString sPassword = password;
    if ((sUser.isEmpty()) && (sPassword.isEmpty()) && (!_pDataSource->m_pImpl->m_sUser.isEmpty()))
    {   // ease the usage of this method. data source which are intended to have a user automatically
        // fill in the user/password combination if the caller of this method does not specify otherwise
        sUser = _pDataSource->m_pImpl->m_sUser;
        if (!_pDataSource->m_pImpl->m_aPassword.isEmpty())
            sPassword = _pDataSource->m_pImpl->m_aPassword;
    }

    ::connectivity::OConnectionWrapper::createUniqueId(url,aInfoCopy,nId.m_pBuffer,sUser,sPassword);
    TConnectionMap::iterator aIter = m_aConnections.find(nId);

    if ( m_aConnections.end() == aIter )
    {
        TConnectionHolder aHolder;
        aHolder.nALiveCount = 0; // will be incremented by addListener
        aHolder.xMasterConnection = _pDataSource->buildIsolatedConnection(user,password);
        aIter = m_aConnections.emplace(nId,aHolder).first;
    }

    Reference<XConnection> xRet;
    if ( aIter->second.xMasterConnection.is() )
    {
        Reference< XAggregation > xConProxy = m_xProxyFactory->createProxy(aIter->second.xMasterConnection.get());
        xRet = new OSharedConnection(xConProxy);
        m_aSharedConnection.emplace(xRet,aIter);
        addEventListener(xRet,aIter);
    }

    return xRet;
}

void OSharedConnectionManager::addEventListener(const Reference<XConnection>& _rxConnection, TConnectionMap::iterator const & _rIter)
{
    Reference<XComponent> xComp(_rxConnection,UNO_QUERY);
    xComp->addEventListener(this);
    OSL_ENSURE( m_aConnections.end() != _rIter , "Iterator is end!");
    osl_atomic_increment(&_rIter->second.nALiveCount);
}

namespace
{
    Sequence< PropertyValue > lcl_filterDriverProperties( const Reference< XDriver >& _xDriver, const OUString& _sUrl,
        const Sequence< PropertyValue >& _rDataSourceSettings, const AsciiPropertyValue* _pKnownSettings )
    {
        if ( _xDriver.is() )
        {
            Sequence< DriverPropertyInfo > aDriverInfo(_xDriver->getPropertyInfo(_sUrl,_rDataSourceSettings));

            const PropertyValue* pDataSourceSetting = _rDataSourceSettings.getConstArray();
            const PropertyValue* pEnd = pDataSourceSetting + _rDataSourceSettings.getLength();

            std::vector< PropertyValue > aRet;

            for ( ; pDataSourceSetting != pEnd ; ++pDataSourceSetting )
            {
                bool bAllowSetting = false;
                const AsciiPropertyValue* pSetting = _pKnownSettings;
                for ( ; pSetting->AsciiName; ++pSetting )
                {
                    if ( pDataSourceSetting->Name.equalsAscii( pSetting->AsciiName ) )
                    {   // the particular data source setting is known

                        const DriverPropertyInfo* pAllowedDriverSetting = aDriverInfo.getConstArray();
                        const DriverPropertyInfo* pDriverSettingsEnd = pAllowedDriverSetting + aDriverInfo.getLength();
                        for ( ; pAllowedDriverSetting != pDriverSettingsEnd; ++pAllowedDriverSetting )
                        {
                            if ( pAllowedDriverSetting->Name.equalsAscii( pSetting->AsciiName ) )
                            {   // the driver also allows this setting
                                bAllowSetting = true;
                                break;
                            }
                        }
                        break;
                    }
                }
                if ( bAllowSetting || !pSetting->AsciiName )
                {   // if the driver allows this particular setting, or if the setting is completely unknown,
                    // we pass it to the driver
                    aRet.push_back( *pDataSourceSetting );
                }
            }
            if ( !aRet.empty() )
                return comphelper::containerToSequence(aRet);
        }
        return Sequence< PropertyValue >();
    }

    typedef std::map< OUString, sal_Int32 > PropertyAttributeCache;

    struct IsDefaultAndNotRemoveable
    {
    private:
        const PropertyAttributeCache& m_rAttribs;

    public:
        explicit IsDefaultAndNotRemoveable( const PropertyAttributeCache& _rAttribs ) : m_rAttribs( _rAttribs ) { }

        bool operator()( const PropertyValue& _rProp )
        {
            if ( _rProp.State != PropertyState_DEFAULT_VALUE )
                return false;

            bool bRemoveable = true;

            PropertyAttributeCache::const_iterator pos = m_rAttribs.find( _rProp.Name );
            OSL_ENSURE( pos != m_rAttribs.end(), "IsDefaultAndNotRemoveable: illegal property name!" );
            if ( pos != m_rAttribs.end() )
                bRemoveable = ( ( pos->second & PropertyAttribute::REMOVABLE ) != 0 );

            return !bRemoveable;
        }
    };
}


ODatabaseSource::ODatabaseSource(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl)
            :ModelDependentComponent( _pImpl )
            ,ODatabaseSource_Base( getMutex() )
            ,OPropertySetHelper( ODatabaseSource_Base::rBHelper )
            , m_Bookmarks(*this, getMutex())
            ,m_aFlushListeners( getMutex() )
{
    // some kind of default
    SAL_INFO("dbaccess", "DS: ctor: " << std::hex << this << ": " << std::hex << m_pImpl.get() );
}

ODatabaseSource::~ODatabaseSource()
{
    SAL_INFO("dbaccess", "DS: dtor: " << std::hex << this << ": " << std::hex << m_pImpl.get() );
    if ( !ODatabaseSource_Base::rBHelper.bInDispose && !ODatabaseSource_Base::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }
}

void ODatabaseSource::setName( const Reference< XDocumentDataSource >& _rxDocument, const OUString& _rNewName, DBContextAccess )
{
    ODatabaseSource& rModelImpl = dynamic_cast< ODatabaseSource& >( *_rxDocument.get() );

    SolarMutexGuard g;
    if ( rModelImpl.m_pImpl.is() )
        rModelImpl.m_pImpl->m_sName = _rNewName;
}

// css::lang::XTypeProvider
Sequence< Type > ODatabaseSource::getTypes()
{
    OTypeCollection aPropertyHelperTypes(   cppu::UnoType<XFastPropertySet>::get(),
                                            cppu::UnoType<XPropertySet>::get(),
                                            cppu::UnoType<XMultiPropertySet>::get());

    return ::comphelper::concatSequences(
        ODatabaseSource_Base::getTypes(),
        aPropertyHelperTypes.getTypes()
    );
}

Sequence< sal_Int8 > ODatabaseSource::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// css::uno::XInterface
Any ODatabaseSource::queryInterface( const Type & rType )
{
    Any aIface = ODatabaseSource_Base::queryInterface( rType );
    if ( !aIface.hasValue() )
        aIface = ::cppu::OPropertySetHelper::queryInterface( rType );
    return aIface;
}

void ODatabaseSource::acquire() throw ()
{
    ODatabaseSource_Base::acquire();
}

void ODatabaseSource::release() throw ()
{
    ODatabaseSource_Base::release();
}

void SAL_CALL ODatabaseSource::disposing( const css::lang::EventObject& Source )
{
    if ( m_pImpl.is() )
        m_pImpl->disposing(Source);
}

// XServiceInfo
OUString ODatabaseSource::getImplementationName(  )
{
    return OUString("com.sun.star.comp.dba.ODatabaseSource");
}

Sequence< OUString > ODatabaseSource::getSupportedServiceNames(  )
{
    return { SERVICE_SDB_DATASOURCE, "com.sun.star.sdb.DocumentDataSource" };
}

sal_Bool ODatabaseSource::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

// OComponentHelper
void ODatabaseSource::disposing()
{
    SAL_INFO("dbaccess", "DS: disp: " << std::hex << this << ", " << std::hex << m_pImpl.get() );
    ODatabaseSource_Base::WeakComponentImplHelperBase::disposing();
    OPropertySetHelper::disposing();

    EventObject aDisposeEvent(static_cast<XWeak*>(this));
    m_aFlushListeners.disposeAndClear( aDisposeEvent );

    ODatabaseDocument::clearObjectContainer(m_pImpl->m_xCommandDefinitions);
    ODatabaseDocument::clearObjectContainer(m_pImpl->m_xTableDefinitions);
    m_pImpl.clear();
}

namespace
{
#if ENABLE_FIREBIRD_SDBC
    weld::Window* GetFrameWeld(const Reference<XModel>& rModel)
    {
        if (!rModel.is())
            return nullptr;
        Reference<XController> xController(rModel->getCurrentController());
        if (!xController.is())
            return nullptr;
        Reference<XFrame> xFrame(xController->getFrame());
        if (!xFrame.is())
            return nullptr;
        Reference<css::awt::XWindow> xWindow(xFrame->getContainerWindow());
        return Application::GetFrameWeld(xWindow);
    }
#endif
}

Reference< XConnection > ODatabaseSource::buildLowLevelConnection(const OUString& _rUid, const OUString& _rPwd)
{
    Reference< XConnection > xReturn;

    Reference< XDriverManager > xManager;

#if ENABLE_FIREBIRD_SDBC
    bool bNeedMigration = false;
    if(m_pImpl->m_sConnectURL == "sdbc:embedded:hsqldb")
    {
        Reference<XStorage> const xRootStorage = m_pImpl->getOrCreateRootStorage();
        OUString sMigrEnvVal;
        osl_getEnvironment(OUString("DBACCESS_HSQL_MIGRATION").pData,
            &sMigrEnvVal.pData);
        if(!sMigrEnvVal.isEmpty())
            bNeedMigration = true;
        else
        {
            Reference<XPropertySet> const xPropSet(xRootStorage, UNO_QUERY_THROW);
            sal_Int32 nOpenMode(0);
            if ((xPropSet->getPropertyValue("OpenMode") >>= nOpenMode)
                && (nOpenMode & css::embed::ElementModes::WRITE))
            {
                MigrationWarnDialog aWarnDlg(GetFrameWeld(m_pImpl->getModel_noCreate()));
                bNeedMigration = aWarnDlg.run() == RET_OK;
            }
        }
        if (bNeedMigration)
        {
            // back up content xml file if migration was successful
            constexpr char BACKUP_XML_NAME[] = "content_before_migration.xml";
            try
            {
                if(xRootStorage->isStreamElement(BACKUP_XML_NAME))
                    xRootStorage->removeElement(BACKUP_XML_NAME);
            }
            catch (NoSuchElementException&)
            {
                SAL_INFO("dbaccess", "No file content_before_migration.xml found" );
            }
            xRootStorage->copyElementTo("content.xml", xRootStorage,
                BACKUP_XML_NAME);

            m_pImpl->m_sConnectURL = "sdbc:embedded:firebird";
        }
    }
#endif

    try {
        xManager.set( ConnectionPool::create( m_pImpl->m_aContext ), UNO_QUERY_THROW );
    } catch( const Exception& ) {  }
    if ( !xManager.is() )
        // no connection pool installed, fall back to driver manager
        xManager.set( DriverManager::create(m_pImpl->m_aContext ), UNO_QUERY_THROW );

    OUString sUser(_rUid);
    OUString sPwd(_rPwd);
    if ((sUser.isEmpty()) && (sPwd.isEmpty()) && (!m_pImpl->m_sUser.isEmpty()))
    {   // ease the usage of this method. data source which are intended to have a user automatically
        // fill in the user/password combination if the caller of this method does not specify otherwise
        sUser = m_pImpl->m_sUser;
        if (!m_pImpl->m_aPassword.isEmpty())
            sPwd = m_pImpl->m_aPassword;
    }

    const char* pExceptionMessageId = RID_STR_COULDNOTCONNECT_UNSPECIFIED;
    if (xManager.is())
    {
        sal_Int32 nAdditionalArgs(0);
        if (!sUser.isEmpty()) ++nAdditionalArgs;
        if (!sPwd.isEmpty()) ++nAdditionalArgs;

        Sequence< PropertyValue > aUserPwd(nAdditionalArgs);
        sal_Int32 nArgPos = 0;
        if (!sUser.isEmpty())
        {
            aUserPwd[ nArgPos ].Name = "user";
            aUserPwd[ nArgPos ].Value <<= sUser;
            ++nArgPos;
        }
        if (!sPwd.isEmpty())
        {
            aUserPwd[ nArgPos ].Name = "password";
            aUserPwd[ nArgPos ].Value <<= sPwd;
        }
        Reference< XDriver > xDriver;
        try
        {

            // choose driver
            Reference< XDriverAccess > xAccessDrivers( xManager, UNO_QUERY );
            if ( xAccessDrivers.is() )
                xDriver = xAccessDrivers->getDriverByURL( m_pImpl->m_sConnectURL );
        }
        catch( const Exception& )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("dbaccess",  "ODatabaseSource::buildLowLevelConnection: got a strange exception while analyzing the error! " << exceptionToString(ex) );
        }
        if ( !xDriver.is() || !xDriver->acceptsURL( m_pImpl->m_sConnectURL ) )
        {
            // Nowadays, it's allowed for a driver to be registered for a given URL, but actually not to accept it.
            // This is because registration nowadays happens at compile time (by adding respective configuration data),
            // but acceptance is decided at runtime.
            pExceptionMessageId = RID_STR_COULDNOTCONNECT_NODRIVER;
        }
        else
        {
            Sequence< PropertyValue > aDriverInfo = lcl_filterDriverProperties(
                xDriver,
                m_pImpl->m_sConnectURL,
                m_pImpl->m_xSettings->getPropertyValues(),
                dbaccess::ODatabaseModelImpl::getDefaultDataSourceSettings()
            );

            if ( m_pImpl->isEmbeddedDatabase() )
            {
                sal_Int32 nCount = aDriverInfo.getLength();
                aDriverInfo.realloc(nCount + 3 );

                aDriverInfo[nCount].Name = "URL";
                aDriverInfo[nCount++].Value <<= m_pImpl->getURL();

                aDriverInfo[nCount].Name = "Storage";
                Reference< css::document::XDocumentSubStorageSupplier> xDocSup( m_pImpl->getDocumentSubStorageSupplier() );
                aDriverInfo[nCount++].Value <<= xDocSup->getDocumentSubStorage("database",ElementModes::READWRITE);

                aDriverInfo[nCount].Name = "Document";
                aDriverInfo[nCount++].Value <<= getDatabaseDocument();
            }
            if (nAdditionalArgs)
                xReturn = xManager->getConnectionWithInfo(m_pImpl->m_sConnectURL, ::comphelper::concatSequences(aUserPwd,aDriverInfo));
            else
                xReturn = xManager->getConnectionWithInfo(m_pImpl->m_sConnectURL,aDriverInfo);

            if ( m_pImpl->isEmbeddedDatabase() )
            {
                // see ODatabaseSource::flushed for comment on why we register as FlushListener
                // at the connection
                Reference< XFlushable > xFlushable( xReturn, UNO_QUERY );
                if ( xFlushable.is() )
                    FlushNotificationAdapter::installAdapter( xFlushable, this );
            }
        }
    }
    else
        pExceptionMessageId = RID_STR_COULDNOTLOAD_MANAGER;

    if ( !xReturn.is() )
    {
        OUString sMessage = DBA_RES(pExceptionMessageId)
            .replaceAll("$name$", m_pImpl->m_sConnectURL);

        SQLContext aContext;
        aContext.Message = DBA_RES(RID_STR_CONNECTION_REQUEST).
            replaceFirst("$name$", m_pImpl->m_sConnectURL);

        throwGenericSQLException( sMessage, static_cast< XDataSource* >( this ), makeAny( aContext ) );
    }

#if ENABLE_FIREBIRD_SDBC
    if( bNeedMigration )
    {
        Reference< css::document::XDocumentSubStorageSupplier> xDocSup(
                m_pImpl->getDocumentSubStorageSupplier() );
        dbahsql::HsqlImporter importer(xReturn,
                xDocSup->getDocumentSubStorage("database",ElementModes::READWRITE) );
        importer.importHsqlDatabase(GetFrameWeld(m_pImpl->getModel_noCreate()));
    }
#endif

    return xReturn;
}

// OPropertySetHelper
Reference< XPropertySetInfo >  ODatabaseSource::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ODatabaseSource::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(13)
        DECL_PROP1(INFO,                        Sequence< PropertyValue >,  BOUND);
        DECL_PROP1_BOOL(ISPASSWORDREQUIRED,                                 BOUND);
        DECL_PROP1_BOOL(ISREADONLY,                                         READONLY);
        DECL_PROP1(LAYOUTINFORMATION,           Sequence< PropertyValue >,  BOUND);
        DECL_PROP1(NAME,                        OUString,            READONLY);
        DECL_PROP2_IFACE(NUMBERFORMATSSUPPLIER, XNumberFormatsSupplier,     READONLY, TRANSIENT);
        DECL_PROP1(PASSWORD,                    OUString,            TRANSIENT);
        DECL_PROP2_IFACE(SETTINGS,              XPropertySet,               BOUND, READONLY);
        DECL_PROP1_BOOL(SUPPRESSVERSIONCL,                                  BOUND);
        DECL_PROP1(TABLEFILTER,                 Sequence< OUString >,BOUND);
        DECL_PROP1(TABLETYPEFILTER,             Sequence< OUString >,BOUND);
        DECL_PROP1(URL,                         OUString,            BOUND);
        DECL_PROP1(USER,                        OUString,            BOUND);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& ODatabaseSource::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool ODatabaseSource::convertFastPropertyValue(Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue )
{
    bool bModified(false);
    if ( m_pImpl.is() )
    {
        switch (nHandle)
        {
            case PROPERTY_ID_TABLEFILTER:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_aTableFilter);
                break;
            case PROPERTY_ID_TABLETYPEFILTER:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_aTableTypeFilter);
                break;
            case PROPERTY_ID_USER:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_sUser);
                break;
            case PROPERTY_ID_PASSWORD:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_aPassword);
                break;
            case PROPERTY_ID_ISPASSWORDREQUIRED:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_bPasswordRequired);
                break;
            case PROPERTY_ID_SUPPRESSVERSIONCL:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_bSuppressVersionColumns);
                break;
            case PROPERTY_ID_LAYOUTINFORMATION:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_aLayoutInformation);
                break;
            case PROPERTY_ID_URL:
            {
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_pImpl->m_sConnectURL);
            }   break;
            case PROPERTY_ID_INFO:
            {
                Sequence<PropertyValue> aValues;
                if (!(rValue >>= aValues))
                    throw IllegalArgumentException();

                for ( auto const & checkName : aValues )
                {
                    if ( checkName.Name.isEmpty() )
                        throw IllegalArgumentException();
                }

                Sequence< PropertyValue > aSettings = m_pImpl->m_xSettings->getPropertyValues();
                bModified = aSettings.getLength() != aValues.getLength();
                if ( !bModified )
                {
                    const PropertyValue* pInfoIter = aSettings.getConstArray();
                    const PropertyValue* checkValue = aValues.getConstArray();
                    for ( ;!bModified && checkValue != aValues.end() ; ++checkValue,++pInfoIter)
                    {
                        bModified = checkValue->Name != pInfoIter->Name;
                        if ( !bModified )
                        {
                            bModified = checkValue->Value != pInfoIter->Value;
                        }
                    }
                }

                rConvertedValue = rValue;
                rOldValue <<= aSettings;
            }
            break;
            default:
                SAL_WARN("dbaccess", "ODatabaseSource::convertFastPropertyValue: unknown or readonly Property!" );
        }
    }
    return bModified;
}

namespace
{
    struct SelectPropertyName
    {
    public:
        const OUString& operator()( const PropertyValue& _lhs )
        {
            return _lhs.Name;
        }
    };

    /** sets a new set of property values for a given property bag instance

        The method takes a property bag, and a sequence of property values to set for this bag.
        Upon return, every property which is not part of the given sequence is
        <ul><li>removed from the bag, if it's a removable property</li>
            <li><em>or</em>reset to its default value, if it's not a removable property</li>
        </ul>.

        @param  _rxPropertyBag
            the property bag to operate on
        @param  _rAllNewPropertyValues
            the new property values to set for the bag
    */
    void lcl_setPropertyValues_resetOrRemoveOther( const Reference< XPropertyBag >& _rxPropertyBag, const Sequence< PropertyValue >& _rAllNewPropertyValues )
    {
        // sequences are ugly to operate on
        std::set<OUString> aToBeSetPropertyNames;
        std::transform(
            _rAllNewPropertyValues.begin(),
            _rAllNewPropertyValues.end(),
            std::inserter( aToBeSetPropertyNames, aToBeSetPropertyNames.end() ),
            SelectPropertyName()
        );

        try
        {
            // obtain all properties currently known at the bag
            Reference< XPropertySetInfo > xPSI( _rxPropertyBag->getPropertySetInfo(), UNO_QUERY_THROW );
            Sequence< Property > aAllExistentProperties( xPSI->getProperties() );

            Reference< XPropertyState > xPropertyState( _rxPropertyBag, UNO_QUERY_THROW );

            // loop through them, and reset resp. default properties which are not to be set
            for ( auto const & existentProperty : aAllExistentProperties )
            {
                if ( aToBeSetPropertyNames.find( existentProperty.Name ) != aToBeSetPropertyNames.end() )
                    continue;

                // this property is not to be set, but currently exists in the bag.
                // -> Remove it, or reset it to the default.
                if ( ( existentProperty.Attributes & PropertyAttribute::REMOVABLE ) != 0 )
                    _rxPropertyBag->removeProperty( existentProperty.Name );
                else
                    xPropertyState->setPropertyToDefault( existentProperty.Name );
            }

            // finally, set the new property values
            _rxPropertyBag->setPropertyValues( _rAllNewPropertyValues );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

void ODatabaseSource::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
{
    if ( m_pImpl.is() )
    {
        switch(nHandle)
        {
            case PROPERTY_ID_TABLEFILTER:
                rValue >>= m_pImpl->m_aTableFilter;
                break;
            case PROPERTY_ID_TABLETYPEFILTER:
                rValue >>= m_pImpl->m_aTableTypeFilter;
                break;
            case PROPERTY_ID_USER:
                rValue >>= m_pImpl->m_sUser;
                // if the user name has changed, reset the password
                m_pImpl->m_aPassword.clear();
                break;
            case PROPERTY_ID_PASSWORD:
                rValue >>= m_pImpl->m_aPassword;
                break;
            case PROPERTY_ID_ISPASSWORDREQUIRED:
                m_pImpl->m_bPasswordRequired = any2bool(rValue);
                break;
            case PROPERTY_ID_SUPPRESSVERSIONCL:
                m_pImpl->m_bSuppressVersionColumns = any2bool(rValue);
                break;
            case PROPERTY_ID_URL:
                rValue >>= m_pImpl->m_sConnectURL;
                break;
            case PROPERTY_ID_INFO:
            {
                Sequence< PropertyValue > aInfo;
                OSL_VERIFY( rValue >>= aInfo );
                lcl_setPropertyValues_resetOrRemoveOther( m_pImpl->m_xSettings, aInfo );
            }
            break;
            case PROPERTY_ID_LAYOUTINFORMATION:
                rValue >>= m_pImpl->m_aLayoutInformation;
                break;
        }
        m_pImpl->setModified(true);
    }
}

void ODatabaseSource::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    if ( m_pImpl.is() )
    {
        switch (nHandle)
        {
            case PROPERTY_ID_TABLEFILTER:
                rValue <<= m_pImpl->m_aTableFilter;
                break;
            case PROPERTY_ID_TABLETYPEFILTER:
                rValue <<= m_pImpl->m_aTableTypeFilter;
                break;
            case PROPERTY_ID_USER:
                rValue <<= m_pImpl->m_sUser;
                break;
            case PROPERTY_ID_PASSWORD:
                rValue <<= m_pImpl->m_aPassword;
                break;
            case PROPERTY_ID_ISPASSWORDREQUIRED:
                rValue <<= m_pImpl->m_bPasswordRequired;
                break;
            case PROPERTY_ID_SUPPRESSVERSIONCL:
                rValue <<= m_pImpl->m_bSuppressVersionColumns;
                break;
            case PROPERTY_ID_ISREADONLY:
                rValue <<= m_pImpl->m_bReadOnly;
                break;
            case PROPERTY_ID_INFO:
            {
                try
                {
                    // collect the property attributes of all current settings
                    Reference< XPropertySet > xSettingsAsProps( m_pImpl->m_xSettings, UNO_QUERY_THROW );
                    Reference< XPropertySetInfo > xPST( xSettingsAsProps->getPropertySetInfo(), UNO_QUERY_THROW );
                    Sequence< Property > aSettings( xPST->getProperties() );
                    std::map< OUString, sal_Int32 > aPropertyAttributes;
                    for ( auto const & setting : aSettings )
                    {
                        aPropertyAttributes[ setting.Name ] = setting.Attributes;
                    }

                    // get all current settings with their values
                    Sequence< PropertyValue > aValues( m_pImpl->m_xSettings->getPropertyValues() );

                    // transform them so that only property values which fulfill certain
                    // criteria survive
                    Sequence< PropertyValue > aNonDefaultOrUserDefined( aValues.getLength() );
                    const PropertyValue* pCopyEnd = std::remove_copy_if(
                        aValues.begin(),
                        aValues.end(),
                        aNonDefaultOrUserDefined.getArray(),
                        IsDefaultAndNotRemoveable( aPropertyAttributes )
                    );
                    aNonDefaultOrUserDefined.realloc( pCopyEnd - aNonDefaultOrUserDefined.getArray() );
                    rValue <<= aNonDefaultOrUserDefined;
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
                }
            }
            break;
            case PROPERTY_ID_SETTINGS:
                rValue <<= m_pImpl->m_xSettings;
                break;
            case PROPERTY_ID_URL:
                rValue <<= m_pImpl->m_sConnectURL;
                break;
            case PROPERTY_ID_NUMBERFORMATSSUPPLIER:
                rValue <<= m_pImpl->getNumberFormatsSupplier();
                break;
            case PROPERTY_ID_NAME:
                rValue <<= m_pImpl->m_sName;
                break;
            case PROPERTY_ID_LAYOUTINFORMATION:
                rValue <<= m_pImpl->m_aLayoutInformation;
                break;
            default:
                SAL_WARN("dbaccess","unknown Property");
        }
    }
}

// XDataSource
void ODatabaseSource::setLoginTimeout(sal_Int32 seconds)
{
    ModelMethodGuard aGuard( *this );
    m_pImpl->m_nLoginTimeout = seconds;
}

sal_Int32 ODatabaseSource::getLoginTimeout()
{
    ModelMethodGuard aGuard( *this );
    return m_pImpl->m_nLoginTimeout;
}

// XCompletedConnection
Reference< XConnection > SAL_CALL ODatabaseSource::connectWithCompletion( const Reference< XInteractionHandler >& _rxHandler )
{
    return connectWithCompletion(_rxHandler,false);
}

Reference< XConnection > ODatabaseSource::getConnection(const OUString& user, const OUString& password)
{
    return getConnection(user,password,false);
}

Reference< XConnection > SAL_CALL ODatabaseSource::getIsolatedConnection( const OUString& user, const OUString& password )
{
    return getConnection(user,password,true);
}

Reference< XConnection > SAL_CALL ODatabaseSource::getIsolatedConnectionWithCompletion( const Reference< XInteractionHandler >& _rxHandler )
{
    return connectWithCompletion(_rxHandler,true);
}

Reference< XConnection > ODatabaseSource::connectWithCompletion( const Reference< XInteractionHandler >& _rxHandler,bool _bIsolated )
{
    ModelMethodGuard aGuard( *this );

    if (!_rxHandler.is())
    {
        SAL_WARN("dbaccess","ODatabaseSource::connectWithCompletion: invalid interaction handler!");
        return getConnection(m_pImpl->m_sUser, m_pImpl->m_aPassword,_bIsolated);
    }

    OUString sUser(m_pImpl->m_sUser), sPassword(m_pImpl->m_aPassword);
    bool bNewPasswordGiven = false;

    if (m_pImpl->m_bPasswordRequired && sPassword.isEmpty())
    {   // we need a password, but don't have one yet.
        // -> ask the user

        // build an interaction request
        // two continuations (Ok and Cancel)
        OInteractionAbort* pAbort = new OInteractionAbort;
        OAuthenticationContinuation* pAuthenticate = new OAuthenticationContinuation;

        // the name which should be referred in the login dialog
        OUString sServerName( m_pImpl->m_sName );
        INetURLObject aURLCheck( sServerName );
        if ( aURLCheck.GetProtocol() != INetProtocol::NotValid )
            sServerName = aURLCheck.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::Unambiguous );

        // the request
        AuthenticationRequest aRequest;
        aRequest.ServerName = sServerName;
        aRequest.HasRealm = aRequest.HasAccount = false;
        aRequest.HasUserName = aRequest.HasPassword = true;
        aRequest.UserName = m_pImpl->m_sUser;
        aRequest.Password = m_pImpl->m_sFailedPassword.isEmpty() ?  m_pImpl->m_aPassword : m_pImpl->m_sFailedPassword;
        OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aRequest));
        Reference< XInteractionRequest > xRequest(pRequest);
        // some knittings
        pRequest->addContinuation(pAbort);
        pRequest->addContinuation(pAuthenticate);

        // handle the request
        try
        {
            _rxHandler->handle(xRequest);
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }

        if (!pAuthenticate->wasSelected())
            return Reference< XConnection >();

        // get the result
        sUser = m_pImpl->m_sUser = pAuthenticate->getUser();
        sPassword = pAuthenticate->getPassword();

        if (pAuthenticate->getRememberPassword())
        {
            m_pImpl->m_aPassword = pAuthenticate->getPassword();
            bNewPasswordGiven = true;
        }
        m_pImpl->m_sFailedPassword.clear();
    }

    try
    {
        return getConnection(sUser, sPassword,_bIsolated);
    }
    catch(Exception&)
    {
        if (bNewPasswordGiven)
        {
            m_pImpl->m_sFailedPassword = m_pImpl->m_aPassword;
            // assume that we had an authentication problem. Without this we may, after an unsuccessful connect, while
            // the user gave us a password an the order to remember it, never allow an password input again (at least
            // not without restarting the session)
            m_pImpl->m_aPassword.clear();
        }
        throw;
    }
}

Reference< XConnection > ODatabaseSource::buildIsolatedConnection(const OUString& user, const OUString& password)
{
    Reference< XConnection > xConn;
    Reference< XConnection > xSdbcConn = buildLowLevelConnection(user, password);
    OSL_ENSURE( xSdbcConn.is(), "ODatabaseSource::buildIsolatedConnection: invalid return value of buildLowLevelConnection!" );
    // buildLowLevelConnection is expected to always succeed
    if ( xSdbcConn.is() )
    {
        // build a connection server and return it (no stubs)
        xConn = new OConnection(*this, xSdbcConn, m_pImpl->m_aContext);
    }
    return xConn;
}

Reference< XConnection > ODatabaseSource::getConnection(const OUString& user, const OUString& password,bool _bIsolated)
{
    ModelMethodGuard aGuard( *this );

    Reference< XConnection > xConn;
    if ( _bIsolated )
    {
        xConn = buildIsolatedConnection(user,password);
    }
    else
    { // create a new proxy for the connection
        if ( !m_pImpl->m_xSharedConnectionManager.is() )
        {
            m_pImpl->m_pSharedConnectionManager = new OSharedConnectionManager( m_pImpl->m_aContext );
            m_pImpl->m_xSharedConnectionManager = m_pImpl->m_pSharedConnectionManager;
        }
        xConn = m_pImpl->m_pSharedConnectionManager->getConnection(
            m_pImpl->m_sConnectURL, user, password, m_pImpl->m_xSettings->getPropertyValues(), this );
    }

    if ( xConn.is() )
    {
        Reference< XComponent> xComp(xConn,UNO_QUERY);
        if ( xComp.is() )
            xComp->addEventListener( static_cast< XContainerListener* >( this ) );
        m_pImpl->m_aConnections.emplace_back(xConn);
    }

    return xConn;
}

Reference< XNameAccess > SAL_CALL ODatabaseSource::getBookmarks(  )
{
    ModelMethodGuard aGuard( *this );
    // tdf#114596 this may look nutty but see OBookmarkContainer::acquire()
    return static_cast<XNameContainer*>(&m_Bookmarks);
}

Reference< XNameAccess > SAL_CALL ODatabaseSource::getQueryDefinitions( )
{
    ModelMethodGuard aGuard( *this );

    Reference< XNameAccess > xContainer = m_pImpl->m_xCommandDefinitions;
    if ( !xContainer.is() )
    {
        Any aValue;
        css::uno::Reference< css::uno::XInterface > xMy(*this);
        if ( dbtools::getDataSourceSetting(xMy,"CommandDefinitions",aValue) )
        {
            OUString sSupportService;
            aValue >>= sSupportService;
            if ( !sSupportService.isEmpty() )
            {
                Sequence<Any> aArgs(1);
                aArgs[0] <<= NamedValue("DataSource",makeAny(xMy));
                xContainer.set( m_pImpl->m_aContext->getServiceManager()->createInstanceWithArgumentsAndContext(sSupportService, aArgs, m_pImpl->m_aContext), UNO_QUERY);
            }
        }
        if ( !xContainer.is() )
        {
            TContentPtr& rContainerData( m_pImpl->getObjectContainer( ODatabaseModelImpl::E_QUERY ) );
            xContainer = new OCommandContainer( m_pImpl->m_aContext, *this, rContainerData, false );
        }
        m_pImpl->m_xCommandDefinitions = xContainer;
    }
    return xContainer;
}

// XTablesSupplier
Reference< XNameAccess >  ODatabaseSource::getTables()
{
    ModelMethodGuard aGuard( *this );

    Reference< XNameAccess > xContainer = m_pImpl->m_xTableDefinitions;
    if ( !xContainer.is() )
    {
        TContentPtr& rContainerData( m_pImpl->getObjectContainer( ODatabaseModelImpl::E_TABLE ) );
        xContainer = new OCommandContainer( m_pImpl->m_aContext, *this, rContainerData, true );
        m_pImpl->m_xTableDefinitions = xContainer;
    }
    return xContainer;
}

void SAL_CALL ODatabaseSource::flush(  )
{
    try
    {
        // SYNCHRONIZED ->
        {
            ModelMethodGuard aGuard( *this );

            typedef ::utl::SharedUNOComponent< XModel, ::utl::CloseableComponent > SharedModel;
            SharedModel xModel( m_pImpl->getModel_noCreate(), SharedModel::NoTakeOwnership );

            if ( !xModel.is() )
                xModel.reset( m_pImpl->createNewModel_deliverOwnership(), SharedModel::TakeOwnership );

            Reference< css::frame::XStorable> xStorable( xModel, UNO_QUERY_THROW );
            xStorable->store();
        }
        // <- SYNCHRONIZED

        css::lang::EventObject aFlushedEvent(*this);
        m_aFlushListeners.notifyEach( &XFlushListener::flushed, aFlushedEvent );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void SAL_CALL ODatabaseSource::flushed( const EventObject& /*rEvent*/ )
{
    ModelMethodGuard aGuard( *this );

    // Okay, this is some hack.
    //
    // In general, we have the problem that embedded databases write into their underlying storage, which
    // logically is one of our sub storage, and practically is a temporary file maintained by the
    // package implementation. As long as we did not commit this storage and our main storage,
    // the changes made by the embedded database engine are not really reflected in the database document
    // file. This is Bad (TM) for a "real" database application - imagine somebody entering some
    // data, and then crashing: For a database application, you would expect that the data still is present
    // when you connect to the database next time.
    //
    // Since this is a conceptual problem as long as we do use those ZIP packages (in fact, we *cannot*
    // provide the desired functionality as long as we do not have a package format which allows O(1) writes),
    // we cannot completely fix this. However, we can relax the problem by committing more often - often
    // enough so that data loss is more seldom, and seldom enough so that there's no noticeable performance
    // decrease.
    //
    // For this, we introduced a few places which XFlushable::flush their connections, and register as
    // XFlushListener at the embedded connection (which needs to provide the XFlushable functionality).
    // Then, when the connection is flushed, we commit both the database storage and our main storage.
    //
    // #i55274#

    OSL_ENSURE( m_pImpl->isEmbeddedDatabase(), "ODatabaseSource::flushed: no embedded database?!" );
    bool bWasModified = m_pImpl->m_bModified;
    m_pImpl->commitEmbeddedStorage();
    m_pImpl->setModified( bWasModified );
}

void SAL_CALL ODatabaseSource::addFlushListener( const Reference< css::util::XFlushListener >& _xListener )
{
    m_aFlushListeners.addInterface(_xListener);
}

void SAL_CALL ODatabaseSource::removeFlushListener( const Reference< css::util::XFlushListener >& _xListener )
{
    m_aFlushListeners.removeInterface(_xListener);
}

void SAL_CALL ODatabaseSource::elementInserted( const ContainerEvent& /*Event*/ )
{
    ModelMethodGuard aGuard( *this );
    if ( m_pImpl.is() )
        m_pImpl->setModified(true);
}

void SAL_CALL ODatabaseSource::elementRemoved( const ContainerEvent& /*Event*/ )
{
    ModelMethodGuard aGuard( *this );
    if ( m_pImpl.is() )
        m_pImpl->setModified(true);
}

void SAL_CALL ODatabaseSource::elementReplaced( const ContainerEvent& /*Event*/ )
{
    ModelMethodGuard aGuard( *this );
    if ( m_pImpl.is() )
        m_pImpl->setModified(true);
}

// XDocumentDataSource
Reference< XOfficeDatabaseDocument > SAL_CALL ODatabaseSource::getDatabaseDocument()
{
    ModelMethodGuard aGuard( *this );

    Reference< XModel > xModel( m_pImpl->getModel_noCreate() );
    if ( !xModel.is() )
        xModel = m_pImpl->createNewModel_deliverOwnership();

    return Reference< XOfficeDatabaseDocument >( xModel, UNO_QUERY_THROW );
}

Reference< XInterface > ODatabaseSource::getThis() const
{
    return *const_cast< ODatabaseSource* >( this );
}

}   // namespace dbaccess

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_dba_ODatabaseSource(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    css::uno::Reference<XInterface> inst(
        DatabaseContext::create(context)->createInstance());
    inst->acquire();
    return inst.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
