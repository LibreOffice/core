/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_
#define _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_

#include "ModelImpl.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
/** === end UNO includes === **/

#include <basic/basicmanagerrepository.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/compbase8.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <boost/shared_ptr.hpp>

// needed for registration
namespace com { namespace sun { namespace star {
    namespace lang
    {
        class XMultiServiceFactory;
        class IllegalArgumentException;
    }
} } }

//........................................................................
namespace dbaccess
{
//........................................................................
class DatabaseDocumentLoader;
//============================================================
//= ODatabaseContext
//============================================================
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    ODatabaseContext_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

typedef ::cppu::WeakComponentImplHelper8    <   ::com::sun::star::lang::XServiceInfo
                                            ,   ::com::sun::star::container::XEnumerationAccess
                                            ,   ::com::sun::star::container::XNameAccess
                                            ,   ::com::sun::star::uno::XNamingService
                                            ,   ::com::sun::star::container::XContainer
                                            ,   ::com::sun::star::lang::XSingleServiceFactory
                                            ,   ::com::sun::star::lang::XUnoTunnel
                                            ,   ::com::sun::star::sdb::XDatabaseRegistrations
                                            >   DatabaseAccessContext_Base;

class ODatabaseContext  :public DatabaseAccessContext_Base
                        ,public ::basic::BasicManagerCreationListener
{
private:
    /** loads the given object from the given URL
    @throws WrappedTargetException
        if an error occurs accessing the URL via the UCB
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > loadObjectFromURL(const ::rtl::OUString& _rName,const ::rtl::OUString& _sURL);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getObject( const ::rtl::OUString& _rURL );

    /** sets all properties which were transient at the data source. e.g. password
        @param  _sURL       The file URL of the data source
        @param  _xObject    The data source itself.
    */
    void setTransientProperties(const ::rtl::OUString& _sURL, ODatabaseModelImpl& _rDataSourceModel );

    /** creates a new data source
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            impl_createNewDataSource();

protected:
    ::osl::Mutex                    m_aMutex;
    ::comphelper::ComponentContext  m_aContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                                    m_xDBRegistrationAggregate;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseRegistrations >
                                    m_xDatabaseRegistrations;

    DECLARE_STL_USTRINGACCESS_MAP( ODatabaseModelImpl*, ObjectCache );
    ObjectCache     m_aDatabaseObjects;

    DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >, PropertyCache );
    PropertyCache   m_aDatasourceProperties;
        // as we hold our data sources weak, we have to cache all properties on the data sources which are
        // transient but stored as long as the session lasts. The database context is the session (as it lives
        // as long as the session does), but the data sources may die before the session does, and then be
        // recreated afterwards. So it's our (the context's) responsibility to store the session-persistent
        // properties.

    ::cppu::OInterfaceContainerHelper       m_aContainerListeners;
    DatabaseDocumentLoader*                 m_pDatabaseDocumentLoader;

public:
    ODatabaseContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    virtual ~ODatabaseContext();


    // OComponentHelper
    virtual void SAL_CALL disposing(void);

    // XSingleServiceFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance(  ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XNamingService
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRegisteredObject( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerObject( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Object ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revokeObject( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XDatabaseRegistrations
    virtual ::sal_Bool SAL_CALL hasRegisteredDatabase( const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getRegistrationNames() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDatabaseLocation( const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerDatabaseLocation( const ::rtl::OUString& Name, const ::rtl::OUString& Location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revokeDatabaseLocation( const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changeDatabaseLocation( const ::rtl::OUString& Name, const ::rtl::OUString& NewLocation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isDatabaseRegistrationReadOnly( const ::rtl::OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addDatabaseRegistrationsListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseRegistrationsListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeDatabaseRegistrationsListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseRegistrationsListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XContainer
    virtual void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    void registerDatabaseDocument( ODatabaseModelImpl& _rModelImpl);
    void revokeDatabaseDocument( const ODatabaseModelImpl& _rModelImpl);
    void databaseDocumentURLChange(const ::rtl::OUString& _sOldName, const ::rtl::OUString& _sNewName);
    void storeTransientProperties( ODatabaseModelImpl& _rModelImpl);
    void appendAtTerminateListener(const ODatabaseModelImpl& _rDataSourceModel);
    void removeFromTerminateListener(const ODatabaseModelImpl& _rDataSourceModel);

private:
    // BasicManagerCreationListener
    virtual void onBasicManagerCreated(
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxForDocument,
        BasicManager& _rBasicManager
    );
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
