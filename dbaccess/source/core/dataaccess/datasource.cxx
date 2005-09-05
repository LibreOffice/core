/*************************************************************************
 *
 *  $RCSfile: datasource.cxx,v $
 *
 *  $Revision: 1.61 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-05 08:58:39 $
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

#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#include "datasource.hxx"
#endif
#ifndef _DBA_CORE_USERINFORMATION_HXX_
#include "userinformation.hxx"
#endif
#ifndef _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_
#include "commandcontainer.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERMANAGER_HPP_
#include <com/sun/star/sdbc/XDriverManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XINTERACTIONSUPPLYAUTHENTICATION_HPP_
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_AUTHENTICATIONREQUEST_HPP_
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _DBA_CORE_CONNECTION_HXX_
#include "connection.hxx"
#endif
#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif
#ifndef DBA_CORE_SHARED_CONNECTION_HXX
#include "SharedConnection.hxx"
#endif
#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTSUBSTORAGESUPPLIER_HPP_
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#endif

#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>

#include <algorithm>

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
using namespace ::vos;
using namespace ::dbtools;
using namespace ::comphelper;
namespace css = ::com::sun::star;

//........................................................................
namespace dbaccess
{
//........................................................................

//============================================================
//= OAuthenticationContinuation
//============================================================
    class OAuthenticationContinuation : public OInteraction< XInteractionSupplyAuthentication >
    {
        sal_Bool    m_bDatasourceReadonly : 1;  // if sal_True, the data source using this continuation
                                                // is readonly, which means that no user can be set and
                                                // the password can't be remembered
        sal_Bool    m_bRemberPassword : 1;      // remember the password for this session ?

        ::rtl::OUString     m_sUser;            // the user
        ::rtl::OUString     m_sPassword;        // the user's password

    public:
        OAuthenticationContinuation(sal_Bool _bReadOnlyDS = sal_False);

        sal_Bool SAL_CALL canSetRealm(  ) throw(RuntimeException);
        void SAL_CALL setRealm( const ::rtl::OUString& Realm ) throw(RuntimeException);
        sal_Bool SAL_CALL canSetUserName(  ) throw(RuntimeException);
        void SAL_CALL setUserName( const ::rtl::OUString& UserName ) throw(RuntimeException);
        sal_Bool SAL_CALL canSetPassword(  ) throw(RuntimeException);
        void SAL_CALL setPassword( const ::rtl::OUString& Password ) throw(RuntimeException);
        Sequence< RememberAuthentication > SAL_CALL getRememberPasswordModes( RememberAuthentication& Default ) throw(RuntimeException);
        void SAL_CALL setRememberPassword( RememberAuthentication Remember ) throw(RuntimeException);
        sal_Bool SAL_CALL canSetAccount(  ) throw(RuntimeException);
        void SAL_CALL setAccount( const ::rtl::OUString& Account ) throw(RuntimeException);
        Sequence< RememberAuthentication > SAL_CALL getRememberAccountModes( RememberAuthentication& Default ) throw(RuntimeException);
        void SAL_CALL setRememberAccount( RememberAuthentication Remember ) throw(RuntimeException);

        ::rtl::OUString getUser() const             { return m_sUser; }
        ::rtl::OUString getPassword() const         { return m_sPassword; }
        sal_Bool        getRememberPassword() const { return m_bRemberPassword; }
    };

    //--------------------------------------------------------------------------
    OAuthenticationContinuation::OAuthenticationContinuation(sal_Bool _bReadOnlyDS)
        :m_bDatasourceReadonly(_bReadOnlyDS)
        ,m_bRemberPassword(sal_True)    // TODO: a meaningfull default
    {
    }

    //--------------------------------------------------------------------------
    sal_Bool SAL_CALL OAuthenticationContinuation::canSetRealm(  ) throw(RuntimeException)
    {
        return sal_False;
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OAuthenticationContinuation::setRealm( const ::rtl::OUString& Realm ) throw(RuntimeException)
    {
        DBG_ERROR("OAuthenticationContinuation::setRealm: not supported!");
    }

    //--------------------------------------------------------------------------
    sal_Bool SAL_CALL OAuthenticationContinuation::canSetUserName(  ) throw(RuntimeException)
    {
        return !m_bDatasourceReadonly;
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OAuthenticationContinuation::setUserName( const ::rtl::OUString& _rUser ) throw(RuntimeException)
    {
        m_sUser = _rUser;
    }

    //--------------------------------------------------------------------------
    sal_Bool SAL_CALL OAuthenticationContinuation::canSetPassword(  ) throw(RuntimeException)
    {
        return sal_True;
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OAuthenticationContinuation::setPassword( const ::rtl::OUString& _rPassword ) throw(RuntimeException)
    {
        m_sPassword = _rPassword;
    }

    //--------------------------------------------------------------------------
    Sequence< RememberAuthentication > SAL_CALL OAuthenticationContinuation::getRememberPasswordModes( RememberAuthentication& _reDefault ) throw(RuntimeException)
    {
        Sequence< RememberAuthentication > aReturn(1);
        _reDefault = aReturn[0] = (m_bDatasourceReadonly ? RememberAuthentication_NO : RememberAuthentication_SESSION);
        return aReturn;
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OAuthenticationContinuation::setRememberPassword( RememberAuthentication _eRemember ) throw(RuntimeException)
    {
        m_bRemberPassword = (RememberAuthentication_NO != _eRemember);
    }

    //--------------------------------------------------------------------------
    sal_Bool SAL_CALL OAuthenticationContinuation::canSetAccount(  ) throw(RuntimeException)
    {
        return sal_False;
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OAuthenticationContinuation::setAccount( const ::rtl::OUString& ) throw(RuntimeException)
    {
        DBG_ERROR("OAuthenticationContinuation::setAccount: not supported!");
    }

    //--------------------------------------------------------------------------
    Sequence< RememberAuthentication > SAL_CALL OAuthenticationContinuation::getRememberAccountModes( RememberAuthentication& _reDefault ) throw(RuntimeException)
    {
        Sequence < RememberAuthentication > aReturn(1);
        aReturn[0] = RememberAuthentication_NO;
        _reDefault = RememberAuthentication_NO;
        return aReturn;
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OAuthenticationContinuation::setRememberAccount( RememberAuthentication Remember ) throw(RuntimeException)
    {
        DBG_ERROR("OAuthenticationContinuation::setRememberAccount: not supported!");
    }

    /** The class OSharedConnectionManager implements a structure to share connections.
        It owns the master connections which will be disposed when the last connection proxy is gone.
    */
    typedef ::cppu::WeakImplHelper1< XEventListener > OConnectionHelper_BASE;
    // need to hold the digest
    struct TDigestHolder
    {
        sal_uInt8 m_pBuffer[RTL_DIGEST_LENGTH_SHA1];
        TDigestHolder()
        {
            m_pBuffer[0] = 0;
        }

    };

    class OSharedConnectionManager : public OConnectionHelper_BASE
    {

         // contains the currently used master connections
        typedef struct
        {
            Reference< XConnection >    xMasterConnection;
            oslInterlockedCount         nALiveCount;
        } TConnectionHolder;

        // the less-compare functor, used for the stl::map
        struct TDigestLess : public ::std::binary_function< TDigestHolder, TDigestHolder, bool>
        {
            bool operator() (const TDigestHolder& x, const TDigestHolder& y) const
            {
                sal_uInt32 i;
                for(i=0;i < RTL_DIGEST_LENGTH_SHA1 && (x.m_pBuffer[i] >= y.m_pBuffer[i]); ++i)
                    ;
                return i < RTL_DIGEST_LENGTH_SHA1;
            }
        };

        typedef ::std::map< TDigestHolder,TConnectionHolder,TDigestLess>        TConnectionMap;      // holds the master connections
        typedef ::std::map< Reference< XConnection >,TConnectionMap::iterator>  TSharedConnectionMap;// holds the shared connections

        ::osl::Mutex                m_aMutex;
        TConnectionMap              m_aConnections;         // remeber the master connection in conjunction with the digest
        TSharedConnectionMap        m_aSharedConnection;    // the shared connections with conjunction with an iterator into the connections map
        Reference< XProxyFactory >  m_xProxyFactory;

    protected:
        ~OSharedConnectionManager();

    public:
        OSharedConnectionManager(const Reference< XMultiServiceFactory >& _rxServiceFactory);

        void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException);
        Reference<XConnection> getConnection(   const rtl::OUString& url,
                                                const rtl::OUString& user,
                                                const rtl::OUString& password,
                                                Sequence< PropertyValue >& _aInfo,
                                                ODatabaseSource* _pDataSource);
        void addEventListener(const Reference<XConnection>& _rxConnection,TConnectionMap::iterator& _rIter);
    };

    DBG_NAME(OSharedConnectionManager)
    OSharedConnectionManager::OSharedConnectionManager(const Reference< XMultiServiceFactory >& _rxServiceFactory)
    {
        DBG_CTOR(OSharedConnectionManager,NULL);
        m_xProxyFactory.set(_rxServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.reflection.ProxyFactory"))),UNO_QUERY);
    }

    OSharedConnectionManager::~OSharedConnectionManager()
    {
        DBG_DTOR(OSharedConnectionManager,NULL);
    }

    void SAL_CALL OSharedConnectionManager::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException)
    {
        MutexGuard aGuard(m_aMutex);
        Reference<XConnection> xConnection(Source.Source,UNO_QUERY);
        TSharedConnectionMap::iterator aFind = m_aSharedConnection.find(xConnection);
        if ( m_aSharedConnection.end() != aFind )
        {
            osl_decrementInterlockedCount(&aFind->second->second.nALiveCount);
            if ( !aFind->second->second.nALiveCount )
            {
                ::comphelper::disposeComponent(aFind->second->second.xMasterConnection);
                m_aConnections.erase(aFind->second);
            }
            m_aSharedConnection.erase(aFind);
        }
    }

    Reference<XConnection> OSharedConnectionManager::getConnection( const rtl::OUString& url,
                                            const rtl::OUString& user,
                                            const rtl::OUString& password,
                                            Sequence< PropertyValue >& _aInfo,
                                            ODatabaseSource* _pDataSource)
    {
        MutexGuard aGuard(m_aMutex);
        TConnectionMap::key_type nId;
        Sequence< PropertyValue > aInfoCopy(_aInfo);
        sal_Int32 nPos = aInfoCopy.getLength();
        aInfoCopy.realloc( nPos + 2 );
        aInfoCopy[nPos].Name      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableFilter"));
        aInfoCopy[nPos++].Value <<= _pDataSource->m_pImpl->m_aTableFilter;
        aInfoCopy[nPos].Name      = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableTypeFilter"));
        aInfoCopy[nPos++].Value <<= _pDataSource->m_pImpl->m_aTableTypeFilter; // #22377# OJ

        ::rtl::OUString sUser = user;
        ::rtl::OUString sPassword = password;
        if ((0 == sUser.getLength()) && (0 == sPassword.getLength()) && (0 != _pDataSource->m_pImpl->m_sUser.getLength()))
        {   // ease the usage of this method. data source which are intended to have a user automatically
            // fill in the user/password combination if the caller of this method does not specify otherwise
            // 86951 - 05/08/2001 - frank.schoenheit@germany.sun.com
            sUser = _pDataSource->m_pImpl->m_sUser;
            if (0 != _pDataSource->m_pImpl->m_aPassword.getLength())
                sPassword = _pDataSource->m_pImpl->m_aPassword;
        }

        ::connectivity::OConnectionWrapper::createUniqueId(url,aInfoCopy,nId.m_pBuffer,sUser,sPassword);
        TConnectionMap::iterator aIter = m_aConnections.find(nId);

        if ( m_aConnections.end() == aIter )
        {
            TConnectionHolder aHolder;
            aHolder.nALiveCount = 0; // will be incremented by addListener
            aHolder.xMasterConnection = _pDataSource->buildIsolatedConnection(user,password);
            aIter = m_aConnections.insert(TConnectionMap::value_type(nId,aHolder)).first;
        }

        Reference<XConnection> xRet;
        if ( aIter->second.xMasterConnection.is() )
        {
            Reference< XAggregation > xConProxy = m_xProxyFactory->createProxy(aIter->second.xMasterConnection.get());
            xRet = new OSharedConnection(xConProxy);
            m_aSharedConnection.insert(TSharedConnectionMap::value_type(xRet,aIter));
            addEventListener(xRet,aIter);
        }

        return xRet;
    }
    void OSharedConnectionManager::addEventListener(const Reference<XConnection>& _rxConnection,TConnectionMap::iterator& _rIter)
    {
        Reference<XComponent> xComp(_rxConnection,UNO_QUERY);
        xComp->addEventListener(this);
        OSL_ENSURE( m_aConnections.end() != _rIter , "Iterator is end!");
        osl_incrementInterlockedCount(&_rIter->second.nALiveCount);
    }

    namespace
    {
        Sequence< PropertyValue > lcl_filterDriverProperties(const Reference< XDriver >& _xDriver,const ::rtl::OUString& _sUrl,const Sequence< PropertyValue >& _rDataSourceSettings)
        {
            if ( _xDriver.is() )
            {
                Sequence< DriverPropertyInfo > aDriverInfo(_xDriver->getPropertyInfo(_sUrl,_rDataSourceSettings));
                const char* pKnownSettings[] = {
                                            // known JDBC settings
                                            "JavaDriverClass",
                                            // known settings for file-based drivers
                                            "Extension", "CharSet", "HeaderLine",
                                            "FieldDelimiter", "StringDelimiter", "DecimalDelimiter",
                                            "ThousandDelimiter", "ShowDeleted",
                                            // known ODBC settings
                                            "SystemDriverSettings", "UseCatalog",
                                            // settings related to auto increment handling
                                            "AutoIncrementCreation", "AutoRetrievingStatement", "IsAutoRetrievingEnabled",
                                            // known Adabas driver setting
                                            "ShutdownDatabase", "DataCacheSizeIncrement", "DataCacheSize",
                                            "ControlUser", "ControlPassword",
                                            // known LDAP driver settings
                                            "HostName", "PortNumber", "BaseDN", "MaxRowCount"
                                            // misc known driver settings
                                            "ParameterNameSubstitution", "AddIndexAppendix",
                                            // known SDB level settings
                                            "IgnoreDriverPrivileges", "NoNameLengthLimit", "AppendTableAliasName",
                                            "EnableSQL92Check", "BooleanComparisonMode", "TableTypeFilterMode",
                                            "RespectDriverResultSetType", "UseSchemaInSelect", "UseCatalogInSelect"
                                        };
                sal_Int32 nKnownSettings = sizeof(pKnownSettings) / sizeof(pKnownSettings[0]);

                const PropertyValue* pDataSourceSetting = _rDataSourceSettings.getConstArray();
                const PropertyValue* pEnd = pDataSourceSetting + _rDataSourceSettings.getLength();

                ::std::vector< PropertyValue > aRet;

                for ( ; pDataSourceSetting != pEnd ; ++pDataSourceSetting )
                {
                    sal_Bool bAllowSetting = sal_False;
                    sal_Int32 i;
                    for ( i=0; i < nKnownSettings; ++i )
                    {
                        if ( !pDataSourceSetting->Name.compareToAscii( pKnownSettings[i] ) )
                        {   // the particular setting is known

                            const DriverPropertyInfo* pAllowedDriverSetting = aDriverInfo.getConstArray();
                            const DriverPropertyInfo* pDriverSettingsEnd = pAllowedDriverSetting + aDriverInfo.getLength();
                            for ( ; pAllowedDriverSetting != pDriverSettingsEnd; ++pAllowedDriverSetting )
                            {
                                if ( !pAllowedDriverSetting->Name.compareToAscii( pKnownSettings[i] ) )
                                {   // the driver also allows this setting
                                    bAllowSetting = sal_True;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    if ( bAllowSetting || ( i == nKnownSettings ) )
                    {   // if the driver allows this particular setting, or if the setting is completely unknown,
                        // we pass it to the driver
                        aRet.push_back( *pDataSourceSetting );
                    }
                }

                return Sequence< PropertyValue >(&(*aRet.begin()),aRet.size());
            }
            return Sequence< PropertyValue >();
        }
    }
//============================================================
//= ODatabaseContext
//============================================================
DBG_NAME(ODatabaseSource)
//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ODatabaseSource()
{
    static OMultiInstanceAutoRegistration< ODatabaseSource > aAutoRegistration;
}
//--------------------------------------------------------------------------
Reference< XInterface > ODatabaseSource_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    Reference< XSingleServiceFactory > xDb(_rxFactory->createInstance(SERVICE_SDB_DATABASECONTEXT),UNO_QUERY);
    return xDb->createInstance(  );
}

//--------------------------------------------------------------------------
ODatabaseSource::ODatabaseSource(const ::rtl::Reference<ODatabaseModelImpl>& _pImpl)
            :OSubComponent(m_aMutex, Reference< XInterface >())
            ,OPropertySetHelper(OComponentHelper::rBHelper)
            ,m_aBookmarks(*this, m_aMutex)
            ,m_pImpl(_pImpl)
            ,m_aFlushListeners(m_aMutex)
{
    // some kind of default
    DBG_CTOR(ODatabaseSource,NULL);
}

//--------------------------------------------------------------------------
ODatabaseSource::~ODatabaseSource()
{
    DBG_DTOR(ODatabaseSource,NULL);
    if ( !OComponentHelper::rBHelper.bInDispose && !OComponentHelper::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }
}
// -----------------------------------------------------------------------------
void ODatabaseSource::setMeAsParent(const Reference< XNameAccess >& _xName)
{
    Reference<XChild> xChild(_xName,UNO_QUERY);
    xChild->setParent(*this);
}
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > ODatabaseSource::getTypes() throw (RuntimeException)
{
    OTypeCollection aPropertyHelperTypes(   ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XMultiPropertySet > *)0 ));

    return ::comphelper::concatSequences(
        ::comphelper::concatSequences(
            OSubComponent::getTypes(),
            aPropertyHelperTypes.getTypes()
        ),
        ODatabaseSource_Base::getTypes()
    );
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODatabaseSource::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any ODatabaseSource::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OSubComponent::queryInterface( rType );
    if (!aIface.hasValue())
    {
        aIface = ODatabaseSource_Base::queryInterface( rType );
        if ( !aIface.hasValue() )
        {
            aIface = ::cppu::queryInterface(
                        rType,
                        static_cast< XPropertySet* >( this ),
                        static_cast< XFastPropertySet* >( this ),
                        static_cast< XMultiPropertySet* >( this ));
        }
    }
    return aIface;
}

//--------------------------------------------------------------------------
void ODatabaseSource::acquire() throw ()
{
    OSubComponent::acquire();
}

//--------------------------------------------------------------------------
void ODatabaseSource::release() throw ()
{
    OSubComponent::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(RuntimeException)
{
    if ( m_pImpl.is() )
        m_pImpl->disposing(Source);
}
// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODatabaseSource::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
rtl::OUString ODatabaseSource::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.dba.ODatabaseSource");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseSource::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
Reference< XInterface > ODatabaseSource::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return ODatabaseSource_CreateInstance(_rxFactory);
}


//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODatabaseSource::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDB_DATASOURCE;
    aSNS[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DocumentDataSource"));
    return aSNS;
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseSource::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}
// OComponentHelper
//------------------------------------------------------------------------------
void ODatabaseSource::disposing()
{
    OSubComponent::disposing();
    OPropertySetHelper::disposing();

    EventObject aDisposeEvent(static_cast<XWeak*>(this));
    m_aFlushListeners.disposeAndClear( aDisposeEvent );
    m_pImpl.clear();
}
//------------------------------------------------------------------------------
Reference< XConnection > ODatabaseSource::buildLowLevelConnection(const ::rtl::OUString& _rUid, const ::rtl::OUString& _rPwd)
{
    Reference< XConnection > xReturn;
    Reference< XDriverManager > xManager(m_pImpl->m_xServiceFactory->createInstance(SERVICE_SDBC_CONNECTIONPOOL)
        , UNO_QUERY);

    ::rtl::OUString sUser(_rUid);
    ::rtl::OUString sPwd(_rPwd);
    if ((0 == sUser.getLength()) && (0 == sPwd.getLength()) && (0 != m_pImpl->m_sUser.getLength()))
    {   // ease the usage of this method. data source which are intended to have a user automatically
        // fill in the user/password combination if the caller of this method does not specify otherwise
        // 86951 - 05/08/2001 - frank.schoenheit@germany.sun.com
        sUser = m_pImpl->m_sUser;
        if (0 != m_pImpl->m_aPassword.getLength())
            sPwd = m_pImpl->m_aPassword;
    }

    sal_uInt16 nExceptionMessageId = RID_STR_COULDNOTCONNECT_UNSPECIFIED;
    if (xManager.is())
    {
        sal_Int32 nAdditionalArgs(0);
        if (sUser.getLength()) ++nAdditionalArgs;
        if (sPwd.getLength()) ++nAdditionalArgs;

        Sequence< PropertyValue > aUserPwd(nAdditionalArgs);
        sal_Int32 nArgPos = 0;
        if (sUser.getLength())
        {
            aUserPwd[ nArgPos ].Name = ::rtl::OUString::createFromAscii("user");
            aUserPwd[ nArgPos ].Value <<= sUser;
            ++nArgPos;
        }
        if (sPwd.getLength())
        {
            aUserPwd[ nArgPos ].Name = ::rtl::OUString::createFromAscii("password");
            aUserPwd[ nArgPos ].Value <<= sPwd;
        }
        Reference< XDriver > xDriver;
        try
        {
            Reference< XDriverAccess > xAccessDrivers( xManager, UNO_QUERY );
            if ( xAccessDrivers.is() )
                xDriver = xAccessDrivers->getDriverByURL( m_pImpl->m_sConnectURL );
        }
        catch( const Exception& )
        {
            DBG_ERROR( "ODatabaseSource::buildLowLevelConnection: got a strange exception while analyzing the error!" );
        }
        if ( !xDriver.is() )
            nExceptionMessageId = RID_STR_COULDNOTCONNECT_NODRIVER;
        else
        {
            Sequence< PropertyValue > aDriverInfo = lcl_filterDriverProperties(xDriver,m_pImpl->m_sConnectURL,m_pImpl->m_aInfo);

            if ( m_pImpl->isEmbeddedDatabase() )
            {
                sal_Int32 nCount = aDriverInfo.getLength();
                aDriverInfo.realloc(nCount + 2 );
                aDriverInfo[nCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
                aDriverInfo[nCount++].Value <<= m_pImpl->getURL();
                aDriverInfo[nCount].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Storage"));
                Reference< css::document::XDocumentSubStorageSupplier> xDocSup( m_pImpl->getDocumentSubStorageSupplier() );
                aDriverInfo[nCount++].Value <<= xDocSup->getDocumentSubStorage(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("database")),ElementModes::READWRITE);
            }
            if (nAdditionalArgs)
                xReturn = xManager->getConnectionWithInfo(m_pImpl->m_sConnectURL, ::comphelper::concatSequences(aUserPwd,aDriverInfo));
            else
                xReturn = xManager->getConnectionWithInfo(m_pImpl->m_sConnectURL,aDriverInfo);
        }
    }
    else
        nExceptionMessageId = RID_STR_COULDNOTLOAD_MANAGER;

    if ( !xReturn.is() )
    {
        ::rtl::OUString sMessage = DBACORE_RESSTRING( nExceptionMessageId );

        SQLContext aContext;
        aContext.Message = m_pImpl->m_sConnectURL;

        throwGenericSQLException( sMessage, static_cast< XDataSource* >( this ), makeAny( aContext ) );
    }

    return xReturn;
}

// OPropertySetHelper
//------------------------------------------------------------------------------
Reference< XPropertySetInfo >  ODatabaseSource::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODatabaseSource::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(12)
        DECL_PROP1(INFO,                        Sequence< PropertyValue >,BOUND);
        DECL_PROP1_BOOL(ISPASSWORDREQUIRED,                                 BOUND);
        DECL_PROP1_BOOL(ISREADONLY,                                         READONLY);
        DECL_PROP1(LAYOUTINFORMATION,           Sequence< ::com::sun::star::beans::PropertyValue >,BOUND);
        DECL_PROP1(NAME,                        ::rtl::OUString,            READONLY);
        DECL_PROP2_IFACE(NUMBERFORMATSSUPPLIER, XNumberFormatsSupplier,     READONLY, TRANSIENT);
        DECL_PROP1(PASSWORD,                    ::rtl::OUString,            TRANSIENT);
        DECL_PROP1_BOOL(SUPPRESSVERSIONCL,                                  BOUND);
        DECL_PROP1(TABLEFILTER,                 Sequence< ::rtl::OUString >,BOUND);
        DECL_PROP1(TABLETYPEFILTER,             Sequence< ::rtl::OUString >,BOUND);
        DECL_PROP1(URL,                         ::rtl::OUString,            BOUND);
        DECL_PROP1(USER,                        ::rtl::OUString,            BOUND);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODatabaseSource::getInfoHelper()
{
    return *getArrayHelper();
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseSource::convertFastPropertyValue(Any & rConvertedValue, Any & rOldValue, sal_Int32 nHandle, const Any& rValue ) throw( IllegalArgumentException  )
{
    sal_Bool bModified(sal_False);
    if ( m_pImpl.is() )
    {
        if (m_pImpl->m_bReadOnly)
            throw IllegalArgumentException();

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

                const PropertyValue* pBegin = aValues.getConstArray();
                const PropertyValue* pEnd   = pBegin + aValues.getLength();
                for (;pBegin != pEnd ; ++pBegin)
                {
                    if ( !pBegin->Name.getLength() )
                        throw IllegalArgumentException();
                }


                bModified = m_pImpl->m_aInfo.getLength() != aValues.getLength();
                if ( !bModified )
                {
                    const PropertyValue* pInfoIter = m_pImpl->m_aInfo.getConstArray();
                    const PropertyValue* pBegin = aValues.getConstArray();
                    const PropertyValue* pEnd   = pBegin + aValues.getLength();
                    for (;!bModified && pBegin != pEnd ; ++pBegin,++pInfoIter)
                    {
                        bModified = pBegin->Name != pInfoIter->Name;
                        if ( !bModified )
                        {
                            bModified = !::comphelper::compare(pBegin->Value,pInfoIter->Value);
                        }
                    }
                }

                rConvertedValue = rValue;
                rOldValue <<= m_pImpl->m_aInfo;
            }   break;
            default:
                DBG_ERROR("unknown Property");
        }
    }
    return bModified;
}

//------------------------------------------------------------------------------
void ODatabaseSource::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
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
                // if the user name changed, reset the password
                m_pImpl->m_aPassword = ::rtl::OUString();
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
                rValue >>= m_pImpl->m_aInfo;
                break;
            case PROPERTY_ID_LAYOUTINFORMATION:
                rValue >>= m_pImpl->m_aLayoutInformation;
                break;
        }
        m_pImpl->setModified(sal_True);
    }
}

//------------------------------------------------------------------------------
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
                rValue = bool2any(m_pImpl->m_bPasswordRequired);
                break;
            case PROPERTY_ID_SUPPRESSVERSIONCL:
                rValue = bool2any(m_pImpl->m_bSuppressVersionColumns);
                break;
            case PROPERTY_ID_ISREADONLY:
                rValue = bool2any(m_pImpl->m_bReadOnly);
                break;
            case PROPERTY_ID_INFO:
                rValue <<= m_pImpl->m_aInfo;
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
                DBG_ERROR("unknown Property");
        }
    }
}

// XDataSource
//------------------------------------------------------------------------------
void ODatabaseSource::setLoginTimeout(sal_Int32 seconds) throw( SQLException, RuntimeException )
{
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    m_pImpl->m_nLoginTimeout = seconds;
}

//------------------------------------------------------------------------------
sal_Int32 ODatabaseSource::getLoginTimeout(void) throw( SQLException, RuntimeException )
{
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    return m_pImpl->m_nLoginTimeout;
}


// XCompletedConnection
//------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODatabaseSource::connectWithCompletion( const Reference< XInteractionHandler >& _rxHandler ) throw(SQLException, RuntimeException)
{
    return connectWithCompletion(_rxHandler,sal_False);
}
// -----------------------------------------------------------------------------
Reference< XConnection > ODatabaseSource::getConnection(const rtl::OUString& user, const rtl::OUString& password) throw( SQLException, RuntimeException )
{
    return getConnection(user,password,sal_False);
}
// -----------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODatabaseSource::getIsolatedConnection( const ::rtl::OUString& user, const ::rtl::OUString& password ) throw(SQLException, RuntimeException)
{
    return getConnection(user,password,sal_True);
}
// -----------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODatabaseSource::getIsolatedConnectionWithCompletion( const Reference< XInteractionHandler >& _rxHandler ) throw(SQLException, RuntimeException)
{
    return connectWithCompletion(_rxHandler,sal_True);
}
// -----------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODatabaseSource::connectWithCompletion( const Reference< XInteractionHandler >& _rxHandler,sal_Bool _bIsolated ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    if (!_rxHandler.is())
    {
        DBG_ERROR("ODatabaseSource::connectWithCompletion: invalid interaction handler!");
        return getConnection(m_pImpl->m_sUser, m_pImpl->m_aPassword,_bIsolated);
    }

    ::rtl::OUString sUser(m_pImpl->m_sUser), sPassword(m_pImpl->m_aPassword);
    sal_Bool bNewPasswordGiven = sal_False;

    if (m_pImpl->m_bPasswordRequired && (0 == sPassword.getLength()))
    {   // we need a password, but don't have one yet.
        // -> ask the user

        // build an interaction request
        // two continuations (Ok and Cancel)
        OInteractionAbort* pAbort = new OInteractionAbort;
        OAuthenticationContinuation* pAuthenticate = new OAuthenticationContinuation(m_pImpl->m_bReadOnly);

        // the request
        AuthenticationRequest aRequest;
        aRequest.ServerName = m_pImpl->m_sName;
        aRequest.HasRealm = aRequest.HasAccount = sal_False;
        aRequest.HasUserName = aRequest.HasPassword = sal_True;
        aRequest.UserName = m_pImpl->m_sUser;
        aRequest.Password = m_pImpl->m_aPassword;
        OInteractionRequest* pRequest = new OInteractionRequest(makeAny(aRequest));
        Reference< XInteractionRequest > xRequest(pRequest);
        // some knittings
        pRequest->addContinuation(pAbort);
        pRequest->addContinuation(pAuthenticate);

        // handle the request
        try
        {
            MutexRelease aRelease(m_aMutex);
                // release the mutex when calling the handler, it may need to lock the SolarMutex
            _rxHandler->handle(xRequest);
        }
        catch(Exception&)
        {
            DBG_ERROR("ODatabaseSource::connectWithCompletion: caught an exception while calling the handler!");
        }

        if (!pAuthenticate->wasSelected())
            return Reference< XConnection >();

        // get the result
        sUser = m_pImpl->m_sUser = pAuthenticate->getUser();
        sPassword = pAuthenticate->getPassword();

        if (pAuthenticate->getRememberPassword())
        {
            m_pImpl->m_aPassword = pAuthenticate->getPassword();
            bNewPasswordGiven = sal_True;
        }
    }

    try
    {
        return getConnection(sUser, sPassword,_bIsolated);
    }
    catch(Exception&)
    {
        if (bNewPasswordGiven)
            // assume that we had an authentication problem. Without this we may, after an unsucessfull connect, while
            // the user gave us a password an the order to remember it, never allow an password input again (at least
            // not without restarting the session)
            m_pImpl->m_aPassword = ::rtl::OUString();
        throw;
    }

    DBG_ERROR("ODatabaseSource::connectWithCompletion: reached the unreacable!");
    return Reference< XConnection >();
}
// -----------------------------------------------------------------------------
Reference< XConnection > ODatabaseSource::buildIsolatedConnection(const rtl::OUString& user, const rtl::OUString& password)
{
    Reference< XConnection > xConn;
    Reference< XConnection > xSdbcConn = buildLowLevelConnection(user, password);
    DBG_ASSERT( xSdbcConn.is(), "ODatabaseSource::buildIsolatedConnection: invalid return value of buildLowLevelConnection!" );
    // buildLowLevelConnection is expected to always succeed
    if ( xSdbcConn.is() )
    {
        // build a connection server and return it (no stubs)
        xConn = new OConnection(*this, xSdbcConn, m_pImpl->m_xServiceFactory);
    }
    return xConn;
}
//------------------------------------------------------------------------------
Reference< XConnection > ODatabaseSource::getConnection(const rtl::OUString& user, const rtl::OUString& password,sal_Bool _bIsolated) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);

    Reference< XConnection > xConn;
    if ( _bIsolated )
    {
        xConn = buildIsolatedConnection(user,password);
    }
    else
    { // create a new proxy for the connection
        if ( !m_pImpl->m_xSharedConnectionManager.is() )
        {
            m_pImpl->m_pSharedConnectionManager = new OSharedConnectionManager(m_pImpl->m_xServiceFactory);
            m_pImpl->m_xSharedConnectionManager = m_pImpl->m_pSharedConnectionManager;
        }
        xConn = m_pImpl->m_pSharedConnectionManager->getConnection(m_pImpl->m_sConnectURL,user,password,m_pImpl->m_aInfo,this);
    }

    if ( xConn.is() )
    {
        Reference< XComponent> xComp(xConn,UNO_QUERY);
        if ( xComp.is() )
            xComp->addEventListener(static_cast< css::lang::XEventListener* >(this));
        m_pImpl->m_aConnections.push_back(OWeakConnection(xConn));
    }

    return xConn;
}

//------------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseSource::getBookmarks(  ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return static_cast< XNameContainer* >(&m_aBookmarks);
}

//------------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseSource::getQueryDefinitions( ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    Reference< XNameAccess > xContainer = m_pImpl->m_xCommandDefinitions;
    if ( !xContainer.is() )
    {
        if ( !m_pImpl->m_aContainer[ODatabaseModelImpl::E_QUERY].get() )
        {
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_QUERY] = TContentPtr(new ODefinitionContainer_Impl);
        }
        xContainer = new OCommandContainer(m_pImpl->m_xServiceFactory,*this,m_pImpl->m_aContainer[ODatabaseModelImpl::E_QUERY],sal_False);
        m_pImpl->m_xCommandDefinitions = xContainer;
    }
    return xContainer;
}
// -----------------------------------------------------------------------------
class OConnectionNotifier //: public ::std::unary_function<OWeakConnection,void>
{
public:
    OConnectionNotifier()
    {
    }

    void operator()(OWeakConnection& _xConnection)
    {
    }
};
// -----------------------------------------------------------------------------
void ODatabaseSource::flushTables()
{
    // flush all tables and queries
    ::std::for_each(m_pImpl->m_aConnections.begin(),m_pImpl->m_aConnections.end(),OConnectionNotifier());
}
//------------------------------------------------------------------------------
// XTablesSupplier
//------------------------------------------------------------------------------
Reference< XNameAccess >  ODatabaseSource::getTables() throw( RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
    Reference< XNameAccess > xContainer = m_pImpl->m_xTableDefinitions;
    if ( !xContainer.is() )
    {
        if ( !m_pImpl->m_aContainer[ODatabaseModelImpl::E_TABLE].get() )
        {
            m_pImpl->m_aContainer[ODatabaseModelImpl::E_TABLE] = TContentPtr(new ODefinitionContainer_Impl);
        }
        xContainer = new OCommandContainer(m_pImpl->m_xServiceFactory,*this,m_pImpl->m_aContainer[ODatabaseModelImpl::E_TABLE]);
        m_pImpl->m_xTableDefinitions = xContainer;
    }
    return xContainer;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::flush(  ) throw (RuntimeException)
{
    try
    {
        ResettableMutexGuard _rGuard(m_aMutex);
        ::connectivity::checkDisposed(OComponentHelper::rBHelper.bDisposed);
        Reference< css::frame::XStorable> xStorable(getModelWithPossibleLeak(),UNO_QUERY);
        if ( xStorable.is() )
            xStorable->store();

        css::lang::EventObject aEvt(*this);
        NOTIFY_LISTERNERS(m_aFlushListeners,XFlushListener,flushed)
    }
    catch(Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::addFlushListener( const Reference< ::com::sun::star::util::XFlushListener >& _xListener ) throw (RuntimeException)
{
    m_aFlushListeners.addInterface(_xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::removeFlushListener( const Reference< ::com::sun::star::util::XFlushListener >& _xListener ) throw (RuntimeException)
{
    m_aFlushListeners.removeInterface(_xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::elementInserted( const ContainerEvent& Event ) throw (RuntimeException)
{
    if ( m_pImpl.is() )
        m_pImpl->setModified(sal_True);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::elementRemoved( const ContainerEvent& Event ) throw (RuntimeException)
{
    if ( m_pImpl.is() )
        m_pImpl->setModified(sal_True);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseSource::elementReplaced( const ContainerEvent& Event ) throw (RuntimeException)
{
    if ( m_pImpl.is() )
        m_pImpl->setModified(sal_True);
}
// -----------------------------------------------------------------------------
Reference< XModel > ODatabaseSource::getModelWithPossibleLeak()
{
    Reference< XModel > xModel;
    if ( m_pImpl.is() )
    {
        xModel = m_pImpl->getModel_noCreate();
        if ( !xModel.is() )
        {
            // In the course of #i50905#, the ownership of a XModel instance was more clearly
            // defined and respected throughout all involved implementations. This place
            // here is the last one where a fix wasn't easily possible within the restrictions
            // which applied to the fix (time frame, risk)
            //
            // There's a pretty large comment in ODatabaseDocument::disconnectController
            // explaining how this dilemma could be solved (which in fact suggests to
            // get completely rid of the "sole ownership" concept, and replace it with
            // shared ownership, and vetoable closing).
            //
            // #i50905# / 2005-06-20 / frank.schoenheit@sun.com
            DBG_ERROR( "ODatabaseSource::getModelWithPossibleLeak: creating a model instance with undefined ownership! Probably a resource leak!" );
            xModel = m_pImpl->createNewModel_deliverOwnership();
        }
    }
    return xModel;
}
// -----------------------------------------------------------------------------
// XDocumentDataSource
Reference< XOfficeDatabaseDocument > SAL_CALL ODatabaseSource::getDatabaseDocument() throw (RuntimeException)
{
    return Reference< XOfficeDatabaseDocument >( getModelWithPossibleLeak(), UNO_QUERY );
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................


