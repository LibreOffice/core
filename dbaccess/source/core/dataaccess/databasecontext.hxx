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

#ifndef _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_
#define _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_

#include "ModelImpl.hxx"

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/uno/XAggregation.hpp>

#include <basic/basicmanagerrepository.hxx>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/compbase3.hxx>
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

namespace dbaccess
{
class DatabaseDocumentLoader;
// ODatabaseContext
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    ODatabaseContext_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

typedef ::cppu::WeakComponentImplHelper3    <   ::com::sun::star::lang::XServiceInfo
                                            ,   ::com::sun::star::sdb::XDatabaseContext
                                            ,   ::com::sun::star::lang::XUnoTunnel
                                            >   DatabaseAccessContext_Base;

class ODatabaseContext  :public DatabaseAccessContext_Base
                        ,public ::basic::BasicManagerCreationListener
{
private:
    /** loads the given object from the given URL
    @throws WrappedTargetException
        if an error occurs accessing the URL via the UCB
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > loadObjectFromURL(const OUString& _rName,const OUString& _sURL);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getObject( const OUString& _rURL );

    /** sets all properties which were transient at the data source. e.g. password
        @param  _sURL       The file URL of the data source
        @param  _xObject    The data source itself.
    */
    void setTransientProperties(const OUString& _sURL, ODatabaseModelImpl& _rDataSourceModel );

    /** creates a new data source
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            impl_createNewDataSource();

protected:
    ::osl::Mutex                    m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                    m_aContext;

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
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XNamingService
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRegisteredObject( const OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerObject( const OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Object ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revokeObject( const OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XDatabaseRegistrations
    virtual ::sal_Bool SAL_CALL hasRegisteredDatabase( const OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getRegistrationNames() throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getDatabaseLocation( const OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL registerDatabaseLocation( const OUString& Name, const OUString& Location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL revokeDatabaseLocation( const OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changeDatabaseLocation( const OUString& Name, const OUString& NewLocation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isDatabaseRegistrationReadOnly( const OUString& Name ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
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
    void databaseDocumentURLChange(const OUString& _sOldName, const OUString& _sNewName);
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

}   // namespace dbaccess

#endif // _DBA_COREDATAACCESS_DATABASECONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
