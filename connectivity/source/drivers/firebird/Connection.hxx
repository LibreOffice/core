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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_CONNECTION_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_CONNECTION_HXX

#include "Blob.hxx"
#include "SubComponent.hxx"

#include <ibase.h>

#include <connectivity/CommonTools.hxx>
#include <connectivity/OSubComponent.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <map>
#include <memory>
#include <OTypeInfo.hxx>
#include <unotools/tempfile.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/DocumentEvent.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/util/XModifiable.hpp>

namespace connectivity
{
    namespace firebird
    {

        typedef ::cppu::WeakComponentImplHelper< css::document::XDocumentEventListener,
                                                 css::lang::XServiceInfo,
                                                 css::sdbc::XConnection,
                                                 css::sdbc::XWarningsSupplier
                                               > Connection_BASE;

        class OStatementCommonBase;
        class FirebirdDriver;
        class ODatabaseMetaData;


        typedef ::std::vector< ::connectivity::OTypeInfo>   TTypeInfoVector;
        typedef std::vector< css::uno::WeakReferenceHelper > OWeakRefArray;

        class Connection : public Connection_BASE,
                            public connectivity::OSubComponent<Connection, Connection_BASE>
        {
            friend class connectivity::OSubComponent<Connection, Connection_BASE>;

            ::osl::Mutex        m_aMutex;

            TTypeInfoVector     m_aTypeInfo;    //  vector containing an entry
                                                                    //  for each row returned by
                                                                    //  DatabaseMetaData.getTypeInfo.
            /** The parent driver that created this connection. */
            ::rtl::Reference<FirebirdDriver>     m_xDriver;

            /** The URL passed to us when opening, i.e. of the form sdbc:* */
            ::rtl::OUString     m_sConnectionURL;
            /**
             * The URL passed to firebird, i.e. either a local file (for a
             * temporary .fdb extracted from a .odb or a normal local file) or
             * a remote url.
             */
            ::rtl::OUString     m_sFirebirdURL;

            /* EMBEDDED MODE DATA */
            /** Denotes that we have a .fdb stored within a .odb file. */
            bool            m_bIsEmbedded;

            /**
             * Handle for the parent DatabaseDocument. We need to notify this
             * whenever any data is written to our temporary database so that
             * the user is able to save this back to the .odb file.
             *
             * Note that this is ONLY set in embedded mode.
             */
            css::uno::Reference< css::util::XModifiable >
                m_xParentDocument;

            /**
             * Handle for the folder within the .odb where we store our .fdb
             * (Only used if m_bIsEmbedded is true).
             */
            css::uno::Reference< css::embed::XStorage >
                m_xEmbeddedStorage;
            /**
             * The temporary folder where we extract the .fdb from a .odb.
             * It is only valid if m_bIsEmbedded is true.
             */
            std::unique_ptr< ::utl::TempFile >  m_pExtractedFDBFile;


            /** We are using an external (local) file */
            bool                m_bIsFile;

            /* CONNECTION PROPERTIES */
            bool                m_bIsAutoCommit;
            bool                m_bIsReadOnly;
            sal_Int32           m_aTransactionIsolation;

            isc_db_handle       m_aDBHandle;
            isc_tr_handle       m_aTransactionHandle;

            css::uno::WeakReference< css::sdbcx::XTablesSupplier>
                                m_xCatalog;
            css::uno::WeakReference< css::sdbc::XDatabaseMetaData >
                                m_xMetaData;
            /** Statements owned by this connection. */
            OWeakRefArray       m_aStatements;

            /**
             * Firebird stores binary collations for indexes on Character based
             * columns, these can be binary-incompatible between different icu
             * version, hence we need to rebuild the indexes when switching icu
             * versions.
             */
            void rebuildIndexes()
                throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception);
            void buildTypeInfo()
                throw (css::sdbc::SQLException);

            /**
             * Creates a new transaction with the desired parameters, if
             * necessary discarding an existing transaction. This has to be done
             * anytime we change the transaction isolation, or autocommiting.
             */
            void setupTransaction()
                throw(css::sdbc::SQLException);
            void disposeStatements();

        /** transform named parameters into unnamed parameters
            @param  _sSQL
                The SQL statement to transform.
            @return
                The new statement with unnamed parameters
        */
        OUString transformPreparedStatement(const OUString& _sSQL);

        public:
            explicit Connection(FirebirdDriver* _pDriver);
            virtual ~Connection();

            void construct( const ::rtl::OUString& url,
                                    const css::uno::Sequence< css::beans::PropertyValue >& info)
            throw(css::sdbc::SQLException,
                  css::uno::RuntimeException,
                  std::exception);

            const OUString& getConnectionURL()  const   {return m_sConnectionURL;}
            bool            isEmbedded()        const   {return m_bIsEmbedded;}
            isc_db_handle&  getDBHandle()               {return m_aDBHandle;}
            isc_tr_handle&  getTransaction()
                throw(css::sdbc::SQLException);

            /**
              * Must be called anytime the underlying database is likely to have
              * changed.
              *
              * This is used to notify the database document of any changes, so
              * that the user is informed of any pending changes needing to be
              * saved.
              */
            void notifyDatabaseModified();

            /**
             * Create a new Blob tied to this connection. Blobs are tied to a
             * transaction and not to a statement, hence the connection should
             * deal with their management.
             */
            css::uno::Reference< css::sdbc::XBlob>
                createBlob(ISC_QUAD* pBlobID)
                throw(css::sdbc::SQLException,
                      css::uno::RuntimeException);

            /**
             * Create and/or connect to the sdbcx Catalog. This is completely
             * unrelated to the SQL "Catalog".
             */
            css::uno::Reference< css::sdbcx::XTablesSupplier >
                createCatalog();

            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            // XConnection
            virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual ::rtl::OUString SAL_CALL nativeSQL( const ::rtl::OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL commit(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL rollback(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isClosed(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setCatalog( const ::rtl::OUString& catalog ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual ::rtl::OUString SAL_CALL getCatalog(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XCloseable
            virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
            // XDocumentEventListener
            virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
            // css.lang.XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_CONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
