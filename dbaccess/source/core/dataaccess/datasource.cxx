/*************************************************************************
 *
 *  $RCSfile: datasource.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-13 16:00:03 $
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
#ifndef _DBA_CORE_CONNECTION_HXX_
#include "connection.hxx"
#endif
#ifndef _DBA_CORE_USERINFORMATION_HXX_
#include "userinformation.hxx"
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERMANAGER_HPP_
#include <com/sun/star/sdbc/XDriverManager.hpp>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif

#ifndef _SVTOOLS_CMDPARSE_HXX
#include <svtools/cmdparse.hxx>
#endif
//#ifndef _UTL_STREAM_WRAPPER_HXX_
//#include <unotools/streamwrap.hxx>
//#endif
#ifndef __SGI_STL_SET
#include <stl/set>
#endif

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;
using namespace ::cppu;
using namespace ::osl;
using namespace ::vos;
using namespace ::svt;
using namespace dbaccess;

// persistent tokens
#define PT_SVFORMATTER      0x0001

DBG_NAME(ODatabaseSource);

//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ODatabaseSource()
{
    static OMultiInstanceAutoRegistration< ODatabaseSource > aAutoRegistration;
}

//--------------------------------------------------------------------------
Reference< XInterface > ODatabaseSource_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ODatabaseSource(_rxFactory));
}

//--------------------------------------------------------------------------
ODatabaseSource::ODatabaseSource(const Reference< XMultiServiceFactory >& _rxFactory)
            :OSubComponent(m_aMutex, Reference< XInterface >())
            ,OConfigurationFlushable(m_aMutex)
            ,OPropertySetHelper(OComponentHelper::rBHelper)
            ,m_nLoginTimeout(0)
            ,m_xServiceFactory(_rxFactory)
            ,m_bReadOnly(sal_False) // we're created as service and have to allow the setting of properties
            ,m_bPasswordRequired(sal_False)
            ,m_aForms(*this, m_aMutex)
            ,m_aReports(*this, m_aMutex)
            ,m_aCommandDefinitions(*this, m_aMutex)
{
    // some kind of default
    m_sConnectURL = ::rtl::OUString::createFromAscii("jdbc:");
}

//--------------------------------------------------------------------------
ODatabaseSource::ODatabaseSource(
                    OWeakObject& _rParent,
                    const Reference< XRegistryKey >& _rxConfigurationRoot,
                    const ::rtl::OUString& _rRegistrationName,
                    const Reference< XMultiServiceFactory >& _rxFactory)
            :OSubComponent(m_aMutex, _rParent)
            ,OConfigurationFlushable(m_aMutex)
            ,OPropertySetHelper(OComponentHelper::rBHelper)
            ,m_nLoginTimeout(0)
            ,m_sName(_rRegistrationName)
            ,m_xServiceFactory(_rxFactory)
            ,m_bReadOnly(sal_True)      // assume readonly for the moment, adjusted below
            ,m_bPasswordRequired(sal_False)
            ,m_aForms(*this, m_aMutex)
            ,m_aReports(*this, m_aMutex)
            ,m_aCommandDefinitions(*this, m_aMutex)
{
    m_xConfigurationNode = _rxConfigurationRoot;

    DBG_CTOR(ODatabaseSource,NULL);
    DBG_ASSERT(m_xConfigurationNode.is(), "ODatabaseSource::ODatabaseSource : use ctor 1 if you can't supply a configuration location at the moment !");
    if (m_xConfigurationNode.is())
        initializeFromConfiguration();
    // adjust our readonly flag
    try
    {
        m_bReadOnly = !m_xConfigurationNode.is() || m_xConfigurationNode->isReadOnly();
    }
    catch (InvalidRegistryException&)
    {
    }
}

//--------------------------------------------------------------------------
ODatabaseSource::~ODatabaseSource()
{
    DBG_DTOR(ODatabaseSource,NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > ODatabaseSource::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XDataSource > *)0 ),
                           ::getCppuType( (const Reference< XUnoTunnel > *)0 ),
                           ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                            OSubComponent::getTypes() );

    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODatabaseSource::getUnoTunnelImplementationId() throw (RuntimeException)
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

//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODatabaseSource::getImplementationId() throw (RuntimeException)
{
    return getUnoTunnelImplementationId();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any ODatabaseSource::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OSubComponent::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ODatabaseSource_Base::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = OConfigurationFlushable::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet* >( this ),
                    static_cast< XFastPropertySet* >( this ),
                    static_cast< XMultiPropertySet* >( this ));
    return aIface;
}

//--------------------------------------------------------------------------
void ODatabaseSource::acquire() throw (RuntimeException)
{
    OSubComponent::acquire();
}

//--------------------------------------------------------------------------
void ODatabaseSource::release() throw (RuntimeException)
{
    OSubComponent::release();
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
    return rtl::OUString::createFromAscii("com.sun.star.sdb.ODatabaseSource");
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
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDB_DATASOURCE;
    return aSNS;
}

//------------------------------------------------------------------------------
sal_Bool ODatabaseSource::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 ODatabaseSource::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() != 16)
        return 0;

    if (0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    if (0 == rtl_compareMemory(OContainerElement::getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)static_cast<OContainerElement*>(this);

    return 0;
}

// OComponentHelper
//------------------------------------------------------------------------------
void ODatabaseSource::disposing()
{
    OPropertySetHelper::disposing();
    OConfigurationFlushable::disposing();

    MutexGuard aGuard(m_aMutex);

//  if (m_xConfigurationNode.is())
//      flush();
        // TODO : we need a mechanism for determining wheter we're modified and need that call or not

    Reference< XConnection > xConn;
    for (OWeakConnectionArray::iterator i = m_aConnections.begin(); m_aConnections.end() != i; i++)
    {
        xConn = *i;
        if (xConn.is())
            xConn->close();
    }
    m_aConnections.clear();
}

//------------------------------------------------------------------------------
Reference< XConnection > ODatabaseSource::buildLowLevelConnection(const ::rtl::OUString& _rUid, const ::rtl::OUString& _rPwd)
{
    Reference< XConnection > xReturn;
    Reference< XDriverManager > xManager(m_xServiceFactory->createInstance(SERVICE_SDBC_DRIVERMANAGER), UNO_QUERY);
    if (xManager.is())
    {
        sal_Int32 nAdditionalArgs(0);
        if (_rUid.getLength()) ++nAdditionalArgs;
        if (_rPwd.getLength()) ++nAdditionalArgs;

        Sequence< PropertyValue > aUserPwd(nAdditionalArgs);
        if (_rUid.getLength())
        {
            aUserPwd[0].Name = ::rtl::OUString::createFromAscii("user");
            aUserPwd[0].Value <<= _rUid;
        }
        if (_rPwd.getLength())
        {
            aUserPwd[1].Name = ::rtl::OUString::createFromAscii("password");
            aUserPwd[1].Value <<= _rPwd;
        }
        if (nAdditionalArgs)
            xReturn = xManager->getConnectionWithInfo(m_sConnectURL, aUserPwd);
        else
            xReturn = xManager->getConnection(m_sConnectURL);
    }

    return xReturn;
}

//------------------------------------------------------------------------------
const Reference< XNumberFormatsSupplier > & ODatabaseSource::getNumberFormatsSupplier()
{
    if (!m_xNumberFormatsSupplier.is())
    {
        // the arguments : the locale of the current user
        UserInformation aUserInfo(m_xServiceFactory);
        Sequence< Any > aArguments(1);
        aArguments.getArray()[0] <<= aUserInfo.getUserLanguage();

        m_xNumberFormatsSupplier = Reference< XNumberFormatsSupplier >(m_xServiceFactory->createInstanceWithArguments(
                ::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatsSupplier"),
                aArguments),
            UNO_QUERY);
        DBG_ASSERT(m_xNumberFormatsSupplier.is(), "ODatabaseSource::getNumberFormatsSupplier : could not instantiate the formats supplier !");
    }
    return m_xNumberFormatsSupplier;
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
    BEGIN_PROPERTY_HELPER(10)
        DECL_PROP0(INFO,                        Sequence< PropertyValue >);
        DECL_PROP1_BOOL(ISPASSWORDREQUIRED,                                 BOUND);
        DECL_PROP1_BOOL(ISREADONLY,                                         READONLY);
        DECL_PROP1(NAME,                        ::rtl::OUString,            READONLY);
        DECL_PROP1_IFACE(NUMBERFORMATSSUPPLIER, XNumberFormatsSupplier,     READONLY);
        DECL_PROP2(PASSWORD,                    ::rtl::OUString,            BOUND, TRANSIENT);
        DECL_PROP1(TABLEFILTER,                 Sequence< ::rtl::OUString >,BOUND);
        DECL_PROP1(TABLETYPEFILTER,             Sequence< ::rtl::OUString >,BOUND);
        DECL_PROP0(URL,                         ::rtl::OUString);
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
    if (m_bReadOnly)
        throw IllegalArgumentException();

    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_TABLEFILTER:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTableFilter);
            break;
        case PROPERTY_ID_TABLETYPEFILTER:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTableTypeFilter);
            break;
        case PROPERTY_ID_USER:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sUser);
            break;
        case PROPERTY_ID_PASSWORD:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aPassword);
            break;
        case PROPERTY_ID_ISPASSWORDREQUIRED:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bPasswordRequired);
            break;
        case PROPERTY_ID_URL:
        {
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sConnectURL);
        }   break;
        case PROPERTY_ID_INFO:
        {
            Sequence<PropertyValue> aValues;
            if (!(rValue >>= aValues))
                throw IllegalArgumentException();

            bModified = sal_True;
                // don't wan't to check the properties, it's seems more expensiv than just to set the same props again

            rConvertedValue = rValue;
            rOldValue <<= m_aInfo;
        }   break;
        default:
            DBG_ERROR("unknown Property");
    }
    return bModified;
}

//------------------------------------------------------------------------------
void ODatabaseSource::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_TABLEFILTER:
            rValue >>= m_aTableFilter;
            break;
        case PROPERTY_ID_TABLETYPEFILTER:
            rValue >>= m_aTableTypeFilter;
            break;
        case PROPERTY_ID_USER:
            rValue >>= m_sUser;
            break;
        case PROPERTY_ID_PASSWORD:
            rValue >>= m_aPassword;
            break;
        case PROPERTY_ID_ISPASSWORDREQUIRED:
            m_bPasswordRequired = any2bool(rValue);
            break;
        case PROPERTY_ID_URL:
            rValue >>= m_sConnectURL;
            break;
        case PROPERTY_ID_INFO:
            rValue >>= m_aInfo;
            break;

    }
}

//------------------------------------------------------------------------------
void ODatabaseSource::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_TABLEFILTER:
            rValue <<= m_aTableFilter;
            break;
        case PROPERTY_ID_TABLETYPEFILTER:
            rValue <<= m_aTableTypeFilter;
            break;
        case PROPERTY_ID_USER:
            rValue <<= m_sUser;
            break;
        case PROPERTY_ID_PASSWORD:
            rValue <<= m_aPassword;
            break;
        case PROPERTY_ID_ISPASSWORDREQUIRED:
            rValue = bool2any(m_bPasswordRequired);
            break;
        case PROPERTY_ID_ISREADONLY:
            rValue = bool2any(m_bReadOnly);
            break;
        case PROPERTY_ID_INFO:
            rValue <<= m_aInfo;
            break;
        case PROPERTY_ID_URL:
            rValue <<= m_sConnectURL;
            break;
        case PROPERTY_ID_NUMBERFORMATSSUPPLIER:
            rValue <<= const_cast<ODatabaseSource*>(this)->getNumberFormatsSupplier();
            break;
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
        default:
            DBG_ERROR("unknown Property");
    }
}

// XDataSource
//------------------------------------------------------------------------------
void ODatabaseSource::setLoginTimeout(sal_Int32 seconds) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    m_nLoginTimeout = seconds;
}

//------------------------------------------------------------------------------
sal_Int32 ODatabaseSource::getLoginTimeout(void) throw( SQLException, RuntimeException )
{
    return m_nLoginTimeout;
}

//------------------------------------------------------------------------------
Reference< XConnection > ODatabaseSource::getConnection(const rtl::OUString& user, const rtl::OUString& password) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_aMutex);
    if (OComponentHelper::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XConnection > xSdbcConn = buildLowLevelConnection(user, password);
    if (!xSdbcConn.is())
        // something went heavily wrong, for instance the driver manager could not be instantiated
        throw RuntimeException();

    Reference< XConnection > xConn;

    // build a connection server and return it (no stubs)
    xConn = new OConnection(*this, xSdbcConn, m_xServiceFactory);
    m_aConnections.push_back(OWeakConnection(xConn));

    return xConn;
}


//------------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseSource::getFormDocuments( ) throw(RuntimeException)
{
    return static_cast< XNameContainer* >(&m_aForms);
}

//------------------------------------------------------------------------------
void ODatabaseSource::flush_NoBroadcast()
{
    flushToConfiguration();
}

//------------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseSource::getReportDocuments( ) throw(RuntimeException)
{
    return static_cast< XNameContainer* >(&m_aReports);
}

//------------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL ODatabaseSource::getQueryDefinitions( ) throw(RuntimeException)
{
    return static_cast< XNameContainer* >(&m_aCommandDefinitions);
}

//------------------------------------------------------------------------------
void ODatabaseSource::inserted(const Reference< XInterface >& _rxContainer, const ::rtl::OUString& _rRegistrationName, const Reference< XRegistryKey >& _rxConfigRoot)
{
    MutexGuard aGuard(m_aMutex);

    DBG_ASSERT(!m_xConfigurationNode.is(), "ODatabaseSource::inserted : you're not allowed to change the location if the current one is valid !");
    DBG_ASSERT(_rxConfigRoot.is(), "ODatabaseSource::inserted : invalid argument (the configuration root) !");
    DBG_ASSERT(_rRegistrationName.getLength() != 0, "ODatabaseSource::inserted : invalid argument (the name) !");
    DBG_ASSERT(!m_xParent.is(), "ODatabaseSource::inserted : already connected to a parent !");

    m_xConfigurationNode = _rxConfigRoot;
    m_xParent = _rxContainer;
    m_sName = _rRegistrationName;

    if (m_xConfigurationNode.is())
        initializeFromConfiguration();

    // adjust our readonly flag
    try
    {
        m_bReadOnly = !m_xConfigurationNode.is() || m_xConfigurationNode->isReadOnly();
    }
    catch (InvalidRegistryException&)
    {
        m_bReadOnly = sal_True;
    }
}

//------------------------------------------------------------------------------
void ODatabaseSource::removed()
{
    MutexGuard aGuard(m_aMutex);
    DBG_ASSERT(m_xParent.is(), "ODatabaseSource::removed : not connected to a parent !");

    // dispose the document containers so they release the documents and the configuration resources
    m_aForms.dispose();
    m_aReports.dispose();
    m_aCommandDefinitions.dispose();

    m_xParent = NULL;
    m_xConfigurationNode = NULL;
    m_sName = ::rtl::OUString();

    m_bReadOnly = sal_False;
}

//------------------------------------------------------------------------------
void ODatabaseSource::initializeDocuments()
{
    // initialize the document containers
    Reference< XRegistryKey > xFormDocuments;
    openKey(m_xConfigurationNode, CONFIGKEY_DBLINK_FORMDOCUMENTS, xFormDocuments, sal_True);
    m_aForms.initialize(xFormDocuments);

    Reference< XRegistryKey > xReportDocuments;
    openKey(m_xConfigurationNode, CONFIGKEY_DBLINK_REPORTDOCUMENTS, xReportDocuments, sal_True);
    m_aReports.initialize(xReportDocuments);

    Reference< XRegistryKey > xCommandDefinitions;
    openKey(m_xConfigurationNode, CONFIGKEY_DBLINK_QUERYDOCUMENTS, xCommandDefinitions, sal_True);
    m_aCommandDefinitions.initialize(xCommandDefinitions);
}

//------------------------------------------------------------------------------
void ODatabaseSource::initializeFromConfiguration()
{
    if (!m_xConfigurationNode.is())
    {
        DBG_ERROR("ODatabaseSource::initializeFromConfiguration : invalid configuration key !");
        return;
    }

    readValue(m_xConfigurationNode, CONFIGKEY_DBLINK_CONNECTURL, m_sConnectURL);
    readValue(m_xConfigurationNode, CONFIGKEY_DBLINK_USER, m_sUser);
    readValue(m_xConfigurationNode, CONFIGKEY_DBLINK_TABLEFILTER, m_aTableFilter);
    readValue(m_xConfigurationNode, CONFIGKEY_DBLINK_TABLETYEFILTER, m_aTableTypeFilter);
    readValue(m_xConfigurationNode, CONFIGKEY_DBLINK_LOGINTIMEOUT, m_nLoginTimeout);

    sal_Bool bTemp; // temporary, needed because m_bPasswordRequired is part of a bit field and we need to transport it as a reference
    readValue(m_xConfigurationNode, CONFIGKEY_DBLINK_PASSWORDREQUIRED, bTemp);
    m_bPasswordRequired = bTemp;

    // the property sequence in m_aInfo
    Reference< XRegistryKey > xInfoKey;
    if (openKey(m_xConfigurationNode, CONFIGKEY_DBLINK_INFO, xInfoKey, sal_False))
    {
        ORegistryLevelEnumeration aEnumInfos(xInfoKey);
        m_aInfo.realloc(aEnumInfos.size());
        PropertyValue* pInfos = m_aInfo.getArray();

        // loop the sub keys
        Reference< XRegistryKey > xCurrent;
        sal_Bool bValid;
        while (aEnumInfos.hasMoreElements())
        {
            xCurrent = aEnumInfos.nextElement();
            RegistryValueType eType(RegistryValueType_NOT_DEFINED);
            bValid = sal_True;
            try
            {
                // dependent on the type, read and store the values
                eType = xCurrent->getValueType();
                pInfos->Name = getShortKeyName(xCurrent);
                switch (eType)
                {
                    case RegistryValueType_STRING:
                        pInfos->Value <<= xCurrent->getStringValue();
                        break;
                    case RegistryValueType_LONG:
                    {
                        // temporary HACK: we may have an additional type information (see flushToConfiguration)
                        sal_Int32 nValue = xCurrent->getLongValue();
                        ::rtl::OUString sTypeInformation;
                        Reference< XRegistryKey > xValueKey;
                        if (readValue(xCurrent, ::rtl::OUString::createFromAscii("TypeInformation"), sTypeInformation))
                        {
                            if (sTypeInformation.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("boolean")))
                            {
                                pInfos->Value = ::cppu::bool2any((sal_Bool)nValue);
                                break;
                            }
                            DBG_ERROR("ODatabaseSource::initializeFromConfiguration: unknown type information!");
                        }
                        pInfos->Value <<= nValue;
                    }
                    break;
                    case RegistryValueType_STRINGLIST:
                        pInfos->Value <<= xCurrent->getStringListValue();
                        break;
                    case RegistryValueType_LONGLIST:
                        pInfos->Value <<= xCurrent->getLongListValue();
                        break;
                    default:
                        DBG_ERROR("ODatabaseSource::initializeFromConfiguration: encountered an unsupported data type!");
                        bValid = sal_False;
                        break;
                }
            }
            catch(Exception&)
            {
                DBG_ERROR("ODatabaseSource::initializeFromConfiguration: error reading the infos!");
                bValid = sal_False;
            }
            if (bValid)
                ++pInfos;
        }
        m_aInfo.realloc(pInfos - m_aInfo.getArray());
    }

    initializeDocuments();
}

//------------------------------------------------------------------------------
void ODatabaseSource::flushDocuments()
{
    m_aForms.flush();
    m_aReports.flush();
    m_aCommandDefinitions.flush();
}


//..............................................................................
// (could have been a template, but SUNPRO5 does not like explicit template arguments in
// function calls (like doSomething< ::rtl::OUString >(aArgument)))
#define WRITE(typeclass, runtimetype, keytype)  \
    case TypeClass_##typeclass: \
    {   \
        runtimetype value;  \
        pInfoValues->Value >>= value;   \
        writeValue(xInfoKey, pInfoValues->Name, static_cast< keytype >(value)); \
        aUsedKeys.insert(pInfoValues->Name);    \
    }   \
    break;  \

//------------------------------------------------------------------------------
void ODatabaseSource::flushToConfiguration()
{
    if (!m_xConfigurationNode.is())
    {
        DBG_ERROR("ODatabaseSource::flushToConfiguration : invalid configuration key !");
        return;
    }

    writeValue(m_xConfigurationNode, CONFIGKEY_DBLINK_CONNECTURL, m_sConnectURL);
    writeValue(m_xConfigurationNode, CONFIGKEY_DBLINK_USER, m_sUser);
    writeValue(m_xConfigurationNode, CONFIGKEY_DBLINK_TABLEFILTER, m_aTableFilter);
    writeValue(m_xConfigurationNode, CONFIGKEY_DBLINK_TABLETYEFILTER, m_aTableTypeFilter);
    writeValue(m_xConfigurationNode, CONFIGKEY_DBLINK_LOGINTIMEOUT, m_nLoginTimeout);
    writeValue(m_xConfigurationNode, CONFIGKEY_DBLINK_PASSWORDREQUIRED, m_bPasswordRequired);

    // write the additional info tags
    Reference< XRegistryKey > xInfoKey;
    if (openKey(m_xConfigurationNode, CONFIGKEY_DBLINK_INFO, xInfoKey, sal_True))
    {
        ::std::set< rtl::OUString > aUsedKeys;

        // stage one: write all currently set info values
        const PropertyValue* pInfoValues = m_aInfo.getConstArray();
        for (sal_Int32 i=0; i<m_aInfo.getLength(); ++i, ++pInfoValues)
        {
            switch (pInfoValues->Value.getValueType().getTypeClass())
            {
                WRITE(STRING, ::rtl::OUString, ::rtl::OUString);
                WRITE(UNSIGNED_SHORT, sal_uInt16, sal_Int16);
                WRITE(UNSIGNED_LONG, sal_uInt32, sal_Int32);
                WRITE(SHORT, sal_Int16, sal_Int16);
                WRITE(LONG, sal_Int32, sal_Int32);
            case TypeClass_BOOLEAN:
            {
                // a temporary HACK as long as we're not configuration based: we write the bool value as long,
                // and a special sub key indicating that it is in fact a bool, not a long
                Reference< XRegistryKey > xValueKey;
                Reference< XRegistryKey > xIndicator;
                if (openKey(xInfoKey, pInfoValues->Name, xValueKey, sal_True) && writeValue(xValueKey, ::rtl::OUString::createFromAscii("TypeInformation"), ::rtl::OUString::createFromAscii("boolean")))
                {
                    writeValue(xInfoKey, pInfoValues->Name, ::cppu::any2bool(pInfoValues->Value));
                    aUsedKeys.insert(pInfoValues->Name);
                }
                else
                    DBG_ERROR("ODatabaseSource::flushToConfiguration: failed to write a boolean!");
            }
            break;
            case TypeClass_SEQUENCE:
            {
                // determine the element type
                TypeDescription aTD(pInfoValues->Value.getValueType());
                typelib_IndirectTypeDescription* pSequenceTD =
                    reinterpret_cast< typelib_IndirectTypeDescription* >(aTD.get());
                DBG_ASSERT(pSequenceTD && pSequenceTD->pType, "ODatabaseSource::flushToConfiguration: invalid sequence type!");

                Type aElementType(pSequenceTD->pType);
                switch (aElementType.getTypeClass())
                {
                    WRITE(STRING, Sequence< ::rtl::OUString >, Sequence< ::rtl::OUString >);
                    WRITE(LONG, Sequence< sal_Int32 >, Sequence< sal_Int32 >);
                    default:
                        DBG_ERROR("ODatabaseSource::flushToConfiguration: unsupported property type!");
                        // TODO: we could save all other sequences, too, by extracting the single elements and
                        // write them separately
                }
            }
            break;
            default:
                DBG_ERROR("ODatabaseSource::flushToConfiguration: unsupported property type!");
                // TODO: maybe we could write structs, too. We a usual registry this would be possible,
                // but the configuration we're going to write into would not support this ...
                // Except we would define templates for all structs known in UNO ... would be possible, but
                // maybe to expensive.
                break;
            }
        }

        // stage two: delete all info values which may be present in the registry, but not used by the current values
        ORegistryLevelEnumeration aEnumInfos(xInfoKey);
        while (aEnumInfos.hasMoreElements())
        {
            ::rtl::OUString sExistentKey = getShortKeyName(aEnumInfos.nextElement());
            if (aUsedKeys.end() == aUsedKeys.find(sExistentKey))
                // the key was not inserted by ourself in the previous stage -> delete it
                deleteKey(xInfoKey, sExistentKey);
        }
    }

    flushDocuments();

    // TODO : flushing of queries/tables ?
}

#if 0
//------------------------------------------------------------------------------
void ODatabaseSource::readUIAspects(const ::vos::ORef< ::store::OStream >& _rStream)
{
    DBG_ASSERT(_rStream.isValid() && _rStream->isOpen(), "ODatabaseSource::readUIAspects : you gave me garbage !");

    OCompatStreamSector aCompat(_rStream, OCompatStreamSector::mode_Read);

    sal_Int32 nTokensUsed(0);
    *_rStream >> nTokensUsed;

    if ((nTokensUsed & PT_SVFORMATTER) != 0)
    {
        OCompatStreamSector aCompat(_rStream, OCompatStreamSector::mode_Read);

        // a object which wraps the storage stream into an SvStream
        OStorageStreamWrapper aSvStreamWrapper(_rStream);

        // an ObjectInputStream
        Reference< XObjectInputStream > xInStream(m_xServiceFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")),
            UNO_QUERY
            );
        Reference< XActiveDataSink > xInDataSink(xInStream, UNO_QUERY);
        DBG_ASSERT(xInDataSink.is(), "ODatabaseSource::readUIAspects : could not instantiate an input stream !");
        if (xInDataSink.is())
        {   // can't load the formatter without that service

            // an uno wrapper for the SvStream
            Reference< XInputStream > xUnoStream = new ::comphelper::OInputStreamWrapper(aSvStreamWrapper);
            xInDataSink->setInputStream(xUnoStream);

            try
            {
                Reference< XPersistObject > xPersistentSupplier = xInStream->readObject();
                Reference< XNumberFormatsSupplier > xSupplier(xPersistentSupplier, UNO_QUERY);
                DBG_ASSERT(xSupplier.is(), "ODatabaseSource::readUIAspects : the object read is no NumberFormatsSupplier !");
                if (xSupplier.is())
                    m_xNumberFormatsSupplier = xSupplier;
            }
            catch(...)
            {
                DBG_ERROR("ODatabaseSource::readUIAspects : could not read the formatter !");
            }
        }
    }
}

//------------------------------------------------------------------------------
void ODatabaseSource::writeUIAspects(const ::vos::ORef< ::store::OStream >& _rStream)
{
    DBG_ASSERT(_rStream.isValid() && _rStream->isOpen(), "ODatabaseSource::writeUIAspects : you gave me garbage !");

    OCompatStreamSector aCompat(_rStream, OCompatStreamSector::mode_Write);
    sal_Int32 nTokensUsed(0);

    Reference< XPersistObject > xPersistentSupplier(m_xNumberFormatsSupplier, UNO_QUERY);
    DBG_ASSERT(xPersistentSupplier.is() || !m_xNumberFormatsSupplier.is(),
        "ODatabaseSource::writeUIAspects : have a formats supplier, but it has no XPersistObject interface !");

    // create an SvStream based on _rStream (we'll need this below)
    OStorageStreamWrapper aSvStreamWrapper(_rStream);
    Reference< XObjectOutputStream > xOutStream;
    Reference< XOutputStream > xUnoStream;
        // will be filled below

    if (xPersistentSupplier.is())
    {   // no need to store the formats supplier if we don't have one, then it can be defaulted on loading

        // first, we need an XObjectOutputStream based on _rStream
        xOutStream = Reference< XObjectOutputStream >(m_xServiceFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")),
            UNO_QUERY
            );
        Reference< XActiveDataSource > xOutDataSource(xOutStream, UNO_QUERY);
        DBG_ASSERT(xOutDataSource.is(), "ODatabaseSource::writeUIAspects : could not instantiate an output stream !");
        if (xOutDataSource.is())
        {   // can't store the formatter without that service

            // an uno wrapper for the SvStream
            xUnoStream = new ::comphelper::OOutputStreamWrapper(aSvStreamWrapper);
            xOutDataSource->setOutputStream(xUnoStream);

            nTokensUsed |= PT_SVFORMATTER;
                // now we have all items for storing the formats supplier
        }
    }

    *_rStream << nTokensUsed;


    if ((nTokensUsed & PT_SVFORMATTER) != 0)
    {
        OCompatStreamSector aCompat(_rStream, OCompatStreamSector::mode_Write);
            // as the formatter is a rather complex object and as it's store and load depend on the
            // availableness of some services, we put this into an own "skippable sector"
        try
        {
            xOutStream->writeObject(xPersistentSupplier);
        }
        catch(...)
        {
            DBG_ERROR("ODatabaseSource::writeUIAspects : could not store the formatter");
        }
    }
}
#endif

