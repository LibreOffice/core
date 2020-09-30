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

#pragma once

#include <sal/config.h>

#include <config_features.h>

#include <map>

#include "ModelImpl.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
#include <com/sun/star/uno/XAggregation.hpp>

#if HAVE_FEATURE_SCRIPTING
#include <basic/basrdll.hxx>
#endif

#include <basic/basicmanagerrepository.hxx>
#include <cppuhelper/compbase.hxx>

// needed for registration
namespace com::sun::star {
    namespace lang
    {
        class XMultiServiceFactory;
        class IllegalArgumentException;
    }
}

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

#if HAVE_FEATURE_SCRIPTING
    BasicDLL m_aBasicDLL;
#endif

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

    ::comphelper::OInterfaceContainerHelper2       m_aContainerListeners;
    rtl::Reference<DatabaseDocumentLoader>         m_xDatabaseDocumentLoader;

public:
    explicit ODatabaseContext( const css::uno::Reference< css::uno::XComponentContext >& );
    virtual ~ODatabaseContext() override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XSingleServiceFactory
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(  ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const css::uno::Sequence< css::uno::Any >& _rArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XNamingService
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRegisteredObject( const OUString& Name ) override;
    virtual void SAL_CALL registerObject( const OUString& Name, const css::uno::Reference< css::uno::XInterface >& Object ) override;
    virtual void SAL_CALL revokeObject( const OUString& Name ) override;

    // XDatabaseRegistrations
    virtual sal_Bool SAL_CALL hasRegisteredDatabase( const OUString& Name ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRegistrationNames() override;
    virtual OUString SAL_CALL getDatabaseLocation( const OUString& Name ) override;
    virtual void SAL_CALL registerDatabaseLocation( const OUString& Name, const OUString& Location ) override;
    virtual void SAL_CALL revokeDatabaseLocation( const OUString& Name ) override;
    virtual void SAL_CALL changeDatabaseLocation( const OUString& Name, const OUString& NewLocation ) override;
    virtual sal_Bool SAL_CALL isDatabaseRegistrationReadOnly( const OUString& Name ) override;
    virtual void SAL_CALL addDatabaseRegistrationsListener( const css::uno::Reference< css::sdb::XDatabaseRegistrationsListener >& Listener ) override;
    virtual void SAL_CALL removeDatabaseRegistrationsListener( const css::uno::Reference< css::sdb::XDatabaseRegistrationsListener >& Listener ) override;

    // XContainer
    virtual void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

    // css::lang::XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
    static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
