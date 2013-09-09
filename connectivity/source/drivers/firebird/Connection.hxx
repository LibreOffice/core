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

#ifndef CONNECTIVITY_FIREBIRD_CONNECTION_HXX
#define CONNECTIVITY_FIREBIRD_CONNECTION_HXX

#include "Blob.hxx"
#include "SubComponent.hxx"

#include <ibase.h>

#include <boost/scoped_ptr.hpp>
#include <connectivity/CommonTools.hxx>
#include <connectivity/OSubComponent.hxx>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/weakref.hxx>
#include <map>
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

namespace connectivity
{
    namespace firebird
    {

        typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::document::XDocumentEventListener,
                                                  ::com::sun::star::lang::XServiceInfo,
                                                  ::com::sun::star::sdbc::XConnection,
                                                  ::com::sun::star::sdbc::XWarningsSupplier
                                                > Connection_BASE;

        class OStatementCommonBase;
        class FirebirdDriver;
        class ODatabaseMetaData;


        typedef ::std::vector< ::connectivity::OTypeInfo>   TTypeInfoVector;
        typedef std::vector< ::com::sun::star::uno::WeakReferenceHelper > OWeakRefArray;

        class Connection : public Connection_BASE,
                            public connectivity::OSubComponent<Connection, Connection_BASE>
        {
            friend class connectivity::OSubComponent<Connection, Connection_BASE>;

             /**
             * Location within the .odb that an embedded .fdb will be stored.
             * Only relevant for embedded dbs.
             */
            static const OUString our_sDBLocation;
        protected:
            ::osl::Mutex                    m_aMutex;
        public:
            ::osl::Mutex&                   getMutex()
                                            { return m_aMutex; }
        protected:




            TTypeInfoVector                         m_aTypeInfo;    //  vector containing an entry
                                                                    //  for each row returned by
                                                                    //  DatabaseMetaData.getTypeInfo.
            /** The parent driver that created this connection. */
            FirebirdDriver*     m_pDriver;

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
            sal_Bool            m_bIsEmbedded;
            /**
             * Handle for the folder within the .odb where we store our .fdb
             * (Only used if m_bIsEmbedded is true).
             */
            ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                m_xEmbeddedStorage;
            /**
             * The temporary folder where we extract the .fdb from a .odb.
             * It is only valid if m_bIsEmbedded is true.
             */
            ::boost::scoped_ptr< ::utl::TempFile >  m_pExtractedFDBFile;


            /* REMOTE CONNECTION DATA */
            ::rtl::OUString     m_sUser;

            /* CONNECTION PROPERTIES */
            sal_Bool            m_bIsAutoCommit;
            sal_Bool            m_bIsReadOnly;
            sal_Int32           m_aTransactionIsolation;

            isc_db_handle       m_aDBHandle;
            isc_tr_handle       m_aTransactionHandle;

            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbcx::XTablesSupplier>
                m_xCatalog;
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData >
                m_xMetaData;
            /** Statements owned by this connection. */
            OWeakRefArray       m_aStatements;

            /**
             * If we are embedded in a .odb we need to listen to Document events
             * in order to save the .fdb back into the .odb.
             */
            void attachAsDocumentListener(const ::rtl::OUString& rStorageURL);

            /**
             * Firebird stores binary collations for indexes on Character based
             * columns, these can be binary-incompatible between different icu
             * version, hence we need to rebuild the indexes when switching icu
             * versions.
             */
            void                rebuildIndexes()
                throw(::com::sun::star::sdbc::SQLException);
            void                buildTypeInfo()
                throw(::com::sun::star::sdbc::SQLException);

            /**
             * Creates a new transaction with the desired parameters, if
             * necessary discarding an existing transaction. This has to be done
             * anytime we change the transaction isolation, or autocommiting.
             */
            void                setupTransaction()
                throw(::com::sun::star::sdbc::SQLException);
            void                disposeStatements();
        public:
            virtual void construct( const ::rtl::OUString& url,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info)
            throw(::com::sun::star::sdbc::SQLException,
                  ::com::sun::star::uno::RuntimeException);

            Connection(FirebirdDriver* _pDriver);
            virtual ~Connection();

            void closeAllStatements () throw( ::com::sun::star::sdbc::SQLException);

            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual void SAL_CALL release() throw();

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            // XConnection
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL nativeSQL( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setCatalog( const ::rtl::OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XDocumentEventListener
            virtual void SAL_CALL documentEventOccured( const ::com::sun::star::document::DocumentEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
            // css.lang.XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

            inline ::rtl::OUString  getUserName()       const { return m_sUser; }
            inline isc_db_handle&    getDBHandle()       { return m_aDBHandle; }
            inline FirebirdDriver*  getDriver()         const { return m_pDriver;}

            ::rtl::OUString         getConnectionURL()  const { return m_sConnectionURL; }
            sal_Bool                isEmbedded()        const { return m_bIsEmbedded; }
            isc_tr_handle&          getTransaction() throw(::com::sun::star::sdbc::SQLException);

            /**
             * Create a new Blob tied to this connection. Blobs are tied to a
             * transaction and not to a statement, hence the connection should
             * deal with their management.
             */
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob>
                createBlob(ISC_QUAD* pBlobID)
                throw(::com::sun::star::sdbc::SQLException,
                      ::com::sun::star::uno::RuntimeException);

            /**
             * Create and/or connect to the sdbcx Catalog. This is completely
             * unrelated to the SQL "Catalog".
             */
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >
                createCatalog();
        };
    }
}
#endif // CONNECTIVITY_SCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
