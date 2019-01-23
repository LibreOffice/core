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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_RESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_RESULTSET_HXX

#include "Statement.hxx"

#include <ibase.h>

#include <connectivity/FValue.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>

#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

namespace connectivity
{
    namespace firebird
    {
        /*
        **  OResultSet
        */
        typedef ::cppu::WeakComponentImplHelper<       css::sdbc::XResultSet,
                                                       css::sdbc::XRow,
                                                       css::sdbc::XResultSetMetaDataSupplier,
                                                       css::util::XCancellable,
                                                       css::sdbc::XCloseable,
                                                       css::sdbc::XColumnLocate,
                                                       css::lang::XServiceInfo> OResultSet_BASE;

        /**
         * This ResultSet does not deal with the management of the SQLDA
         * it is supplied with. The owner must mange its SQLDA appropriately
         * and ensure that the ResultSet is destroyed before disposing of the
         * SQLDA.
         */
        class OResultSet: public OResultSet_BASE,
                          public ::comphelper::OPropertyContainer,
                          public  ::comphelper::OPropertyArrayUsageHelper<OResultSet>
        {
        private:
            bool m_bIsBookmarkable;
            sal_Int32 m_nFetchSize;
            sal_Int32 m_nResultSetType;
            sal_Int32 m_nFetchDirection;
            sal_Int32 m_nResultSetConcurrency;

        protected:
            // Connection kept alive by m_xStatement
            Connection* m_pConnection;
            ::osl::Mutex& m_rMutex;
            const css::uno::Reference< css::uno::XInterface >& m_xStatement;

            css::uno::Reference< css::sdbc::XResultSetMetaData>        m_xMetaData;

            XSQLDA*                                     m_pSqlda;
            isc_stmt_handle                             m_statementHandle;

            bool                                        m_bWasNull;
            // Row numbering starts with 0 for "in front of first row"
            sal_Int32                                   m_currentRow;
            bool                                        m_bIsAfterLastRow;

            const sal_Int32                             m_fieldCount;
            ISC_STATUS_ARRAY                            m_statusVector;

            bool isNull(const sal_Int32 nColumnIndex);

            template <typename T> OUString makeNumericString(
                    const sal_Int32 nColumnIndex);

            template <typename T> T     retrieveValue(const sal_Int32 nColumnIndex,
                                                      const ISC_SHORT nType);

            template <typename T> T safelyRetrieveValue(
                    const sal_Int32 nColumnIndex,
                    const ISC_SHORT nType = 0);

            // OIdPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void checkColumnIndex( sal_Int32 index );
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void checkRowIndex();

            // you can't delete objects of this type
            virtual ~OResultSet() override;
        public:
            DECLARE_SERVICE_INFO();

            OResultSet(Connection* pConnection,
                       ::osl::Mutex& rMutex,
                       const css::uno::Reference< css::uno::XInterface >& xStatement,
                       const isc_stmt_handle& aStatementHandle,
                       XSQLDA* aSqlda
                      );

            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface(
                    const css::uno::Type& rType) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
            // XResultSet
            virtual sal_Bool SAL_CALL next(  ) override;
            virtual sal_Bool SAL_CALL isBeforeFirst(  ) override;
            virtual sal_Bool SAL_CALL isAfterLast(  ) override;
            virtual sal_Bool SAL_CALL isFirst(  ) override;
            virtual sal_Bool SAL_CALL isLast(  ) override;
            virtual void SAL_CALL beforeFirst(  ) override;
            virtual void SAL_CALL afterLast(  ) override;
            virtual sal_Bool SAL_CALL first(  ) override;
            virtual sal_Bool SAL_CALL last(  ) override;
            virtual sal_Int32 SAL_CALL getRow(  ) override;
            virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) override;
            virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) override;
            virtual sal_Bool SAL_CALL previous(  ) override;
            virtual void SAL_CALL refreshRow(  ) override;
            virtual sal_Bool SAL_CALL rowUpdated(  ) override;
            virtual sal_Bool SAL_CALL rowInserted(  ) override;
            virtual sal_Bool SAL_CALL rowDeleted(  ) override;
            virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement(  ) override;
            // XRow
            virtual sal_Bool SAL_CALL wasNull(  ) override;
            virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) override;
            virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) override;
            virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) override;
            virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) override;
            virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) override;
            virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) override;
            virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) override;
            virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) override;
            virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) override;
            virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) override;
            virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) override;
            virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) override;
            virtual css::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
            virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) override;
            virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) override;
            // XResultSetMetaDataSupplier
            virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) override;
            // XCloseable
            virtual void SAL_CALL close(  ) override;
            // XWarningsSupplier

            // XColumnLocate
            virtual sal_Int32 SAL_CALL findColumn(const OUString& columnName) override;

        };

        // Specialisations have to be in the namespace and can't be within the class.
        template <> css::util::Date
            OResultSet::retrieveValue(
                const sal_Int32 nColumnIndex,
                const ISC_SHORT nType);
        template <> ::connectivity::ORowSetValue
            OResultSet::retrieveValue(
                const sal_Int32 nColumnIndex,
                const ISC_SHORT nType);
        template <> css::util::Time
            OResultSet::retrieveValue(
                const sal_Int32 nColumnIndex,
                const ISC_SHORT nType);
        template <> css::util::DateTime
            OResultSet::retrieveValue(
                const sal_Int32 nColumnIndex,
                const ISC_SHORT nType);
        template <> ISC_QUAD*
             OResultSet::retrieveValue(
                 const sal_Int32 nColumnIndex,
                 const ISC_SHORT nType);
        template <> OUString
            OResultSet::retrieveValue(
                const sal_Int32 nColumnIndex,
                const ISC_SHORT nType);
        template <> ISC_QUAD*
             OResultSet::retrieveValue(
                 const sal_Int32 nColumnIndex,
                 const ISC_SHORT nType);
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_FIREBIRD_RESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
