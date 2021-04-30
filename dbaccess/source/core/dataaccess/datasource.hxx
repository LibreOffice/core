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

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdbc/XIsolatedConnection.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <cppuhelper/propshlp.hxx>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include <apitools.hxx>
#include <bookmarkcontainer.hxx>
#include <rtl/ref.hxx>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <ContentHelper.hxx>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <ModelImpl.hxx>

namespace dbaccess
{

class OSharedConnectionManager;

// ODatabaseSource
typedef ::cppu::WeakComponentImplHelper<   css::lang::XServiceInfo
                                       ,   css::sdbc::XDataSource
                                       ,   css::sdb::XBookmarksSupplier
                                       ,   css::sdb::XQueryDefinitionsSupplier
                                       ,   css::sdb::XCompletedConnection
                                       ,   css::container::XContainerListener
                                       ,   css::sdbc::XIsolatedConnection
                                       ,   css::sdbcx::XTablesSupplier
                                       ,   css::util::XFlushable
                                       ,   css::util::XFlushListener
                                       ,   css::sdb::XDocumentDataSource
                                       ,   css::lang::XInitialization
                                       >   ODatabaseSource_Base;

class ODatabaseSource   :public ModelDependentComponent // must be first
                        ,public ODatabaseSource_Base
                        ,public ::cppu::OPropertySetHelper
                        ,public ::comphelper::OPropertyArrayUsageHelper < ODatabaseSource >
{
    friend class ODatabaseContext;
    friend class OConnection;
    friend class OSharedConnectionManager;

private:
    using ODatabaseSource_Base::rBHelper;
    // note: this thing uses the ref-count of "this", see OBookmarkContainer::acquire!
    OBookmarkContainer m_Bookmarks;
    ::comphelper::OInterfaceContainerHelper2 m_aFlushListeners;

private:
    virtual ~ODatabaseSource() override;

public:
    explicit ODatabaseSource( const ::rtl::Reference< ODatabaseModelImpl >& _pImpl );

    struct DBContextAccess { friend class ODatabaseContext; private: DBContextAccess() { } };

    /** sets a new name for the data source

        The name of a data source (our m_sName member) is the registration name, *if* the
        data source actually *is* registered at the database context.

        Normally, this name is passed at time of creation of the ODatabaseModelImpl instance,
        but if a newly created data source is registered, then it must be possible to propagate
        the new registration name.
    */
    static void setName(
            const css::uno::Reference< css::sdb::XDocumentDataSource >& _rxDocument,
            const OUString& _rNewName,
            DBContextAccess
        );

    // XContainerListener
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;
    // css::sdbcx::XTablesSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) override;

// css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

// css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

// css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

// OComponentHelper
    virtual void SAL_CALL disposing() override;

// css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

// comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

// cppu::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            css::uno::Any & rConvertedValue,
                            css::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const css::uno::Any& rValue ) override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const css::uno::Any& rValue
                                                 ) override;
    virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

// css::sdb::XCompletedConnection
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connectWithCompletion( const css::uno::Reference< css::task::XInteractionHandler >& handler ) override;

// css::sdbc::XDataSource
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection( const OUString& user, const OUString& password ) override;
    virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) override;
    virtual sal_Int32 SAL_CALL getLoginTimeout(  ) override;

//::css::sdb::XBookmarksSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getBookmarks(  ) override;

//::css::sdb::XQueryDefinitionsSupplier
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getQueryDefinitions(  ) override;

// css::sdbc::XIsolatedConnection
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getIsolatedConnection( const OUString& user, const OUString& password ) override;
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getIsolatedConnectionWithCompletion( const css::uno::Reference< css::task::XInteractionHandler >& handler ) override;

// XFlushable
    virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL addFlushListener( const css::uno::Reference< css::util::XFlushListener >& l ) override;
    virtual void SAL_CALL removeFlushListener( const css::uno::Reference< css::util::XFlushListener >& l ) override;

    // XFlushListener
    virtual void SAL_CALL flushed( const css::lang::EventObject& rEvent ) override;

    // XDocumentDataSource
    virtual css::uno::Reference< css::sdb::XOfficeDatabaseDocument > SAL_CALL getDatabaseDocument() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

protected:
    // ModelDependentComponent overridables
    virtual css::uno::Reference< css::uno::XInterface > getThis() const override;

private:
// helper
    /** open a connection for the current settings. this is the simple connection we get from the driver
        manager, so it can be used as a master for a "high level" sdb connection.
    */
    css::uno::Reference< css::sdbc::XConnection > buildLowLevelConnection(
        const OUString& _rUid, const OUString& _rPwd
        );

    css::uno::Reference< css::sdbc::XConnection > buildIsolatedConnection(
        const OUString& user, const OUString& password
        );

    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::sdbc::XConnection > getConnection( const OUString& user, const OUString& password , bool _bIsolated);
    /// @throws css::sdbc::SQLException
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::sdbc::XConnection > connectWithCompletion( const css::uno::Reference< css::task::XInteractionHandler >& handler , bool _bIsolated);

protected:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
