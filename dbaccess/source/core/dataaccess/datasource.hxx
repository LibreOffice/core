/*************************************************************************
 *
 *  $RCSfile: datasource.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:40 $
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
#define _DBA_COREDATAACCESS_DATASOURCE_HXX_

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _UNOTOOLS_PROPERTY_ARRAY_HELPER_HXX_
#include <unotools/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_REGISTRATION_HELPER_HXX_
#include "registrationhelper.hxx"
#endif
#ifndef _DBA_COREDATAACCESS_DOCUMENTCONTAINER_HXX_
#include "documentcontainer.hxx"
#endif
#ifndef _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_
#include "commandcontainer.hxx"
#endif
#ifndef _DBA_CORE_CONTAINERELEMENT_HXX_
#include "containerelement.hxx"
#endif
#ifndef _DBA_CORE_CONFIGURATIONFLUSHABLE_HXX_
#include "configurationflushable.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif

namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormatsSupplier;
    class XNumberFormatter;
} } } }

typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection > OWeakConnection;
typedef std::vector< OWeakConnection > OWeakConnectionArray;

//============================================================
//= ODatabaseSource
//============================================================
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    ODatabaseSource_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

typedef ::cppu::ImplHelper6 <   ::com::sun::star::lang::XServiceInfo,
                                ::com::sun::star::lang::XUnoTunnel,
                                ::com::sun::star::sdbc::XDataSource,
                                ::com::sun::star::sdb::XFormDocumentsSupplier,
                                ::com::sun::star::sdb::XReportDocumentsSupplier,
                                ::com::sun::star::sdb::XQueryDefinitionsSupplier
                            >   ODatabaseSource_Base;

class ODatabaseSource   :public connectivity::OBaseMutex
                    ,public OSubComponent
                    ,public OContainerElement
                    ,public OConfigurationFlushable
                    ,public ::cppu::OPropertySetHelper
                    ,public ::utl::OPropertyArrayUsageHelper < ODatabaseSource >
                    ,public ODatabaseSource_Base
{
    friend class ODatabaseContext;
    friend class OConnection;
    friend ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        ODatabaseSource_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

#ifdef _PRIVATE_TEST_
    friend void _cdecl main( int argc, char * argv[] );
#endif

protected:
    OWeakConnectionArray        m_aConnections;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                                        m_xServiceFactory;

    ODocumentContainer      m_aForms;
    ODocumentContainer      m_aReports;
    OCommandContainer       m_aCommandDefinitions;

// <properties>
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                                                        m_xNumberFormatsSupplier;
    ::rtl::OUString                                     m_sConnectURL;
    ::rtl::OUString                                     m_sName;        // transient, our creator has to tell us the title
    ::rtl::OUString                                     m_sUser;
    ::rtl::OUString                                     m_aPassword;    // transient !
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableTypeFilter;
    sal_Int32                                           m_nLoginTimeout;
    sal_Bool                                            m_bReadOnly : 1;
    sal_Bool                                            m_bPasswordRequired : 1;
// </properties>

protected:
    ODatabaseSource(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

public:
    ODatabaseSource(
        ::cppu::OWeakObject& _rParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rxConfigurationRoot,
        const ::rtl::OUString& _rRegistrationName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);
    virtual ~ODatabaseSource();

// com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

// com::sun::star::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId() throw (::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing(void);

// com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

// utl::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

// cppu::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )
                                                 throw (::com::sun::star::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

// ::com::sun::star::sdbc::XDataSource
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const ::rtl::OUString& user, const ::rtl::OUString& password ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getLoginTimeout(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XFormDocumentsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFormDocuments(  ) throw(::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XReportDocumentsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getReportDocuments(  ) throw(::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XQueryDefinitionsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getQueryDefinitions(  ) throw(::com::sun::star::uno::RuntimeException);

protected:
    // OConfigurationFlushable
    virtual void flush_NoBroadcast();

protected:
// helper
    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >&
            getNumberFormatsSupplier();

    static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > toConnectionProperties(const String& rConnectStr);
    static rtl::OUString toConnectionURL(const String& rConnectStr);
    static String toConnectionStr(const rtl::OUString& rUrl, const String& rConnectStr = String()) throw (::com::sun::star::sdbc::SQLException);
    static String toConnectionStr(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProperties);

    /** open a connection for the current settings. this is the simple connection we get from the driver
        manager, so it acn be used as a master for a "high level" sdb connection.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > buildLowLevelConnection(
        const ::rtl::OUString& _rUid, const ::rtl::OUString& _rPwd
        );


// OContainerElement
    virtual void        inserted(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContainer,
        const ::rtl::OUString& _rElementName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rxConfigRoot);

    virtual void        removed();

    virtual sal_Bool    isContainerElement() const { return m_xConfigurationNode.is(); }

// other stuff
    void    initializeFromConfiguration();
    void    flushToConfiguration();

#if 0
    void    readUIAspects(const ::vos::ORef< ::store::OStream >& _rStream);
    void    writeUIAspects(const ::vos::ORef< ::store::OStream >& _rStream);
#endif

    void    initializeDocuments();
    void    flushDocuments();
};

#endif // _DBA_COREDATAACCESS_DATALINK_HXX_

