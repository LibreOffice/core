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

#include <ibase.h>

#include <connectivity/CommonTools.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <memory>
#include <OTypeInfo.hxx>
#include <unotools/tempfile.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/DocumentEvent.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XBlob.hpp>
#include <com/sun/star/sdbc/XClob.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/util/XModifiable.hpp>

namespace connectivity::firebird
    {

        typedef ::cppu::WeakComponentImplHelper< css::document::XDocumentEventListener,
                                                 css::lang::XServiceInfo,
                                                 css::sdbc::XConnection,
                                                 css::sdbc::XWarningsSupplier
                                               > Connection_BASE;

        class OStatementCommonBase;
        class FirebirdDriver;
        class ODatabaseMetaData;


        typedef std::vector< ::connectivity::OTypeInfo>   TTypeInfoVector;
        typedef std::vector< css::uno::WeakReferenceHelper > OWeakRefArray;

        class Connection final : public Connection_BASE
        {
            ::osl::Mutex        m_aMutex;

            TTypeInfoVector     m_aTypeInfo;    //  vector containing an entry
                                                                    //  for each row returned by
                                                                    //  DatabaseMetaData.getTypeInfo.

            /** The URL passed to us when opening, i.e. of the form sdbc:* */
            OUString     m_sConnectionURL;
            /**
             * The URL passed to firebird, i.e. either a local file (for a
             * temporary .fdb extracted from a .odb or a normal local file) or
             * a remote url.
             */
            OUString     m_sFirebirdURL;

            /* EMBEDDED MODE DATA */
            /** Denotes that we have a database stored within a .odb file. */
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
             * Handle for the folder within the .odb where we store our .fbk
             * (Only used if m_bIsEmbedded is true).
             */
            css::uno::Reference< css::embed::XStorage >
                m_xEmbeddedStorage;
            /**
             * The temporary folder where we extract the .fbk from a .odb,
             * and also store the temporary .fdb
             * It is only valid if m_bIsEmbedded is true.
             *
             * The extracted .fbk is written in firebird.fbk, the temporary
             * .fdb is stored as firebird.fdb.
             */
            std::unique_ptr< ::utl::TempFile >  m_pDatabaseFileDir;
            /**
             * Path for our extracted .fbk file.
             *
             * (The temporary .fdb is our m_sFirebirdURL.)
             */
            OUString m_sFBKPath;

            void loadDatabaseFile(const OUString& pSrcLocation, const OUString& pTmpLocation);

            /**
             * Run the backup service, use nAction =
             * isc_action_svc_backup to backup, nAction = isc_action_svc_restore
             * to restore.
             */
            void runBackupService(const short nAction);

            isc_svc_handle attachServiceManager();

            void detachServiceManager(isc_svc_handle pServiceHandle);

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

            /// @throws css::sdbc::SQLException
            void buildTypeInfo();

            /**
             * Creates a new transaction with the desired parameters, if
             * necessary discarding an existing transaction. This has to be done
             * anytime we change the transaction isolation, or autocommitting.
             *
             * @throws css::sdbc::SQLException
             */
            void setupTransaction();
            void disposeStatements();

        public:
            explicit Connection();
            virtual ~Connection() override;

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void construct( const OUString& url,
                                    const css::uno::Sequence< css::beans::PropertyValue >& info);

            const OUString& getConnectionURL()  const   {return m_sConnectionURL;}
            bool            isEmbedded()        const   {return m_bIsEmbedded;}
            isc_db_handle&  getDBHandle()               {return m_aDBHandle;}
            /// @throws css::sdbc::SQLException
            isc_tr_handle&  getTransaction();

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
             *
             * @throws css::sdbc::SQLException
             * @throws css::uno::RuntimeException
             */
            css::uno::Reference< css::sdbc::XBlob>
                createBlob(ISC_QUAD const * pBlobID);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            css::uno::Reference< css::sdbc::XClob>
                createClob(ISC_QUAD const * pBlobID);

            /**
             * Create and/or connect to the sdbcx Catalog. This is completely
             * unrelated to the SQL "Catalog".
             */
            css::uno::Reference< css::sdbcx::XTablesSupplier >
                createCatalog();

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            // XConnection
            virtual css::uno::Reference< css::sdbc::XStatement > SAL_CALL createStatement(  ) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) override;
            virtual css::uno::Reference< css::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) override;
            virtual OUString SAL_CALL nativeSQL( const OUString& sql ) override;
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) override;
            virtual sal_Bool SAL_CALL getAutoCommit(  ) override;
            virtual void SAL_CALL commit(  ) override;
            virtual void SAL_CALL rollback(  ) override;
            virtual sal_Bool SAL_CALL isClosed(  ) override;
            virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) override;
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) override;
            virtual sal_Bool SAL_CALL isReadOnly(  ) override;
            virtual void SAL_CALL setCatalog( const OUString& catalog ) override;
            virtual OUString SAL_CALL getCatalog(  ) override;
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) override;
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) override;
            virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTypeMap(  ) override;
            virtual void SAL_CALL setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
            // XCloseable
            virtual void SAL_CALL close(  ) override;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;
            // XDocumentEventListener
            virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) override;
            // css.lang.XEventListener
            virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
