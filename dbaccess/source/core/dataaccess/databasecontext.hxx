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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DATABASECONTEXT_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DATABASECONTEXT_HXX

#include <sal/config.h>

#include <map>

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
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>

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

typedef ::cppu::WeakComponentImplHelper<   css::lang::XServiceInfo
                                       ,   css::sdb::XDatabaseContext
                                       ,   css::lang::XUnoTunnel
                                       >   DatabaseAccessContext_Base;

class ODatabaseContext  :public DatabaseAccessContext_Base
                        ,public ::basic::BasicManagerCreationListener
{
private:
    /** loads the given object from the given URL
    @throws WrappedTargetException
        if an error occurs accessing the URL via the UCB
    */
    css::uno::Reference< css::uno::XInterface > loadObjectFromURL(const OUString& _rName,const OUString& _sURL);
    css::uno::Reference< css::uno::XInterface > getObject( const OUString& _rURL );

    /** sets all properties which were transient at the data source. e.g. password
        @param  _sURL       The file URL of the data source
        @param  _xObject    The data source itself.
    */
    void setTransientProperties(const OUString& _sURL, ODatabaseModelImpl& _rDataSourceModel );

    /** creates a new data source
    */
    css::uno::Reference< css::uno::XInterface >
            impl_createNewDataSource();

protected:
    ::osl::Mutex                    m_aMutex;
    css::uno::Reference< css::uno::XComponentContext >
                                    m_aContext;

    css::uno::Reference< css::uno::XAggregation >
                                    m_xDBRegistrationAggregate;
    css::uno::Reference< css::sdb::XDatabaseRegistrations >
                                    m_xDatabaseRegistrations;

    typedef std::map<OUString, ODatabaseModelImpl*> ObjectCache;
    ObjectCache     m_aDatabaseObjects;

    typedef std::map< OUString, css::uno::Sequence< css::beans::PropertyValue > > PropertyCache;
    PropertyCache   m_aDatasourceProperties;
        // as we hold our data sources weak, we have to cache all properties on the data sources which are
        // transient but stored as long as the session lasts. The database context is the session (as it lives
        // as long as the session does), but the data sources may die before the session does, and then be
        // recreated afterwards. So it's our (the context's) responsibility to store the session-persistent
        // properties.

    ::cppu::OInterfaceContainerHelper       m_aContainerListeners;
    DatabaseDocumentLoader*                 m_pDatabaseDocumentLoader;

public:
    explicit ODatabaseContext( const css::uno::Reference< css::uno::XComponentContext >& );
    virtual ~ODatabaseContext();

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XSingleServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(  ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const css::uno::Sequence< css::uno::Any >& _rArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo - static methods
    static css::uno::Sequence< OUString > getSupportedServiceNames_static() throw( css::uno::RuntimeException );
    static OUString getImplementationName_static() throw( css::uno::RuntimeException );
    static css::uno::Reference< css::uno::XInterface >
        SAL_CALL Create(const css::uno::Reference< css::uno::XComponentContext >&);

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XNamingService
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRegisteredObject( const OUString& Name ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL registerObject( const OUString& Name, const css::uno::Reference< css::uno::XInterface >& Object ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL revokeObject( const OUString& Name ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XDatabaseRegistrations
    virtual sal_Bool SAL_CALL hasRegisteredDatabase( const OUString& Name ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRegistrationNames() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getDatabaseLocation( const OUString& Name ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL registerDatabaseLocation( const OUString& Name, const OUString& Location ) throw (css::lang::IllegalArgumentException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL revokeDatabaseLocation( const OUString& Name ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL changeDatabaseLocation( const OUString& Name, const OUString& NewLocation ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isDatabaseRegistrationReadOnly( const OUString& Name ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addDatabaseRegistrationsListener( const css::uno::Reference< css::sdb::XDatabaseRegistrationsListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDatabaseRegistrationsListener( const css::uno::Reference< css::sdb::XDatabaseRegistrationsListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

    // XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;
    static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

    void registerDatabaseDocument( ODatabaseModelImpl& _rModelImpl);
    void revokeDatabaseDocument( const ODatabaseModelImpl& _rModelImpl);
    void databaseDocumentURLChange(const OUString& _sOldName, const OUString& _sNewName);
    void storeTransientProperties( ODatabaseModelImpl& _rModelImpl);
    void appendAtTerminateListener(const ODatabaseModelImpl& _rDataSourceModel);
    void removeFromTerminateListener(const ODatabaseModelImpl& _rDataSourceModel);

private:
    // BasicManagerCreationListener
    virtual void onBasicManagerCreated(
        const css::uno::Reference< css::frame::XModel >& _rxForDocument,
        BasicManager& _rBasicManager
    ) override;
};

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DATABASECONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
