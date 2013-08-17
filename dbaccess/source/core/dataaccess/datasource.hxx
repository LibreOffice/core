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

#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#define _DBA_COREDATAACCESS_DATASOURCE_HXX_

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
#include <cppuhelper/compbase11.hxx>
#include <com/sun/star/embed/XTransactionListener.hpp>
#include "apitools.hxx"
#include "bookmarkcontainer.hxx"
#include <rtl/ref.hxx>
#include <connectivity/CommonTools.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include "ContentHelper.hxx"
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include "ModelImpl.hxx"

namespace dbaccess
{

class OSharedConnectionManager;

// ODatabaseSource
typedef ::cppu::WeakComponentImplHelper11   <   ::com::sun::star::lang::XServiceInfo
                                            ,   ::com::sun::star::sdbc::XDataSource
                                            ,   ::com::sun::star::sdb::XBookmarksSupplier
                                            ,   ::com::sun::star::sdb::XQueryDefinitionsSupplier
                                            ,   ::com::sun::star::sdb::XCompletedConnection
                                            ,   ::com::sun::star::container::XContainerListener
                                            ,   ::com::sun::star::sdbc::XIsolatedConnection
                                            ,   ::com::sun::star::sdbcx::XTablesSupplier
                                            ,   ::com::sun::star::util::XFlushable
                                            ,   ::com::sun::star::util::XFlushListener
                                            ,   ::com::sun::star::sdb::XDocumentDataSource
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
    OBookmarkContainer                      m_aBookmarks;
    ::cppu::OInterfaceContainerHelper       m_aFlushListeners;

private:
    virtual ~ODatabaseSource();

public:
    ODatabaseSource( const ::rtl::Reference< ODatabaseModelImpl >& _pImpl );

    struct DBContextAccess { friend class ODatabaseContext; private: DBContextAccess() { } };

    /** sets a new name for the data source

        The name of a data source (our m_sName member) is the registration name, *if* the
        data source actually *is* registered at the database context.

        Normally, this name is passed at time of creation of the ODatabaseModelImpl instance,
        but if a newly creaed data source is registered, then it must be possible to propagate
        the new trgistration name.
    */
    static void setName(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDocumentDataSource >& _rxDocument,
            const OUString& _rNewName,
            DBContextAccess
        );

    // XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::sdbcx::XTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);

// com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw( );
    virtual void SAL_CALL release() throw( );

// ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

// OComponentHelper
    virtual void SAL_CALL disposing(void);

// com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// comphelper::OPropertyArrayUsageHelper
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

// ::com::sun::star::sdb::XCompletedConnection
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connectWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XDataSource
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const OUString& user, const OUString& password ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getLoginTimeout(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XBookmarksSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getBookmarks(  ) throw (::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XQueryDefinitionsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getQueryDefinitions(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XIsolatedConnection
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getIsolatedConnection( const OUString& user, const OUString& password ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getIsolatedConnectionWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// XFlushable
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);

    // XFlushListener
    virtual void SAL_CALL flushed( const ::com::sun::star::lang::EventObject& rEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XDocumentDataSource
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XOfficeDatabaseDocument > SAL_CALL getDatabaseDocument() throw (::com::sun::star::uno::RuntimeException);

protected:
    // ModelDependentComponent overridables
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getThis() const;

private:
// helper
    /** open a connection for the current settings. this is the simple connection we get from the driver
        manager, so it can be used as a master for a "high level" sdb connection.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > buildLowLevelConnection(
        const OUString& _rUid, const OUString& _rPwd
        );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > buildIsolatedConnection(
        const OUString& user, const OUString& password
        );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const OUString& user, const OUString& password , sal_Bool _bIsolated) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connectWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler , sal_Bool _bIsolated) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void clearConnections();

protected:
    using ::cppu::OPropertySetHelper::getFastPropertyValue;
};

}   // namespace dbaccess

#endif // _DBA_COREDATAACCESS_DATALINK_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
