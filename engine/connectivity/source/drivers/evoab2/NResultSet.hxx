/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <memory>

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/FValue.hxx>
#include <connectivity/warningscontainer.hxx>
#include "NStatement.hxx"
#include "NResultSetMetaData.hxx"

namespace connectivity::evoab
{
    struct ComparisonData;

    class OEvoabVersionHelper
    {
    public:
        virtual EBook* openBook(const char *abname) = 0;
        virtual void executeQuery (EBook* pBook, EBookQuery* pQuery) = 0;
        virtual void freeContacts() = 0;
        virtual bool isLDAP( EBook *pBook ) = 0;
        virtual bool isLocal( EBook *pBook ) = 0;
        virtual EContact *getContact(sal_Int32 nIndex) = 0;
        virtual sal_Int32 getNumContacts() = 0;
        virtual bool hasContacts() = 0;
        virtual void sortContacts( const ComparisonData& _rCompData ) = 0;
        OString getUserName( EBook *pBook );
        virtual ~OEvoabVersionHelper() {}
    };

    typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XResultSet
                                           ,   css::sdbc::XRow
                                           ,   css::sdbc::XResultSetMetaDataSupplier
                                           ,   css::util::XCancellable
                                           ,   css::sdbc::XWarningsSupplier
                                           ,   css::sdbc::XCloseable
                                           ,   css::sdbc::XColumnLocate
                                           ,   css::lang::XServiceInfo
                                           >   OResultSet_BASE;


    class OEvoabResultSet final : public cppu::BaseMutex
                            ,public OResultSet_BASE
                            ,public ::comphelper::OPropertyContainer
                            ,public ::comphelper::OPropertyArrayUsageHelper<OEvoabResultSet>
    {
    private:
        std::unique_ptr<OEvoabVersionHelper> m_pVersionHelper;

        OCommonStatement*                           m_pStatement;
        OEvoabConnection*                           m_pConnection;
        rtl::Reference<OEvoabResultSetMetaData>     m_xMetaData;
        ::dbtools::WarningsContainer                m_aWarnings;

        bool                                        m_bWasNull;
        // <properties>
        sal_Int32                                   m_nFetchSize;
        sal_Int32                                   m_nResultSetType;
        sal_Int32                                   m_nFetchDirection;
        sal_Int32                                   m_nResultSetConcurrency;
        // </properties>

        // Data & iteration
        sal_Int32 m_nIndex;
        sal_Int32 m_nLength;
        EContact *getCur()
        {
            return m_pVersionHelper->getContact(m_nIndex);
        }

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & getInfoHelper() override;

        // you can't delete objects of this type
        virtual ~OEvoabResultSet() override;
    public:
        DECLARE_SERVICE_INFO();

        OEvoabResultSet( OCommonStatement *pStmt, OEvoabConnection *pConnection );
        void construct( const QueryData& _rData );

        // ::cppu::OComponentHelper
        virtual void disposing() override;
        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;
        //XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
        // XResultSet
        virtual bool next(  ) override;
        virtual bool isBeforeFirst(  ) override;
        virtual bool isAfterLast(  ) override;
        virtual bool isFirst(  ) override;
        virtual bool isLast(  ) override;
        virtual void beforeFirst(  ) override;
        virtual void afterLast(  ) override;
        virtual bool first(  ) override;
        virtual bool last(  ) override;
        virtual sal_Int32 getRow(  ) override;
        virtual bool absolute( sal_Int32 row ) override;
        virtual bool relative( sal_Int32 rows ) override;
        virtual bool previous(  ) override;
        virtual void refreshRow(  ) override;
        virtual bool rowUpdated(  ) override;
        virtual bool rowInserted(  ) override;
        virtual bool rowDeleted(  ) override;
        virtual css::uno::Reference< css::uno::XInterface > getStatement(  ) override;
        // XRow
        virtual bool wasNull(  ) override;
        virtual OUString getString( sal_Int32 columnIndex ) override;
        virtual bool getBoolean( sal_Int32 columnIndex ) override;
        virtual sal_Int8 getByte( sal_Int32 columnIndex ) override;
        virtual sal_Int16 getShort( sal_Int32 columnIndex ) override;
        virtual sal_Int32 getInt( sal_Int32 columnIndex ) override;
        virtual sal_Int64 getLong( sal_Int32 columnIndex ) override;
        virtual float getFloat( sal_Int32 columnIndex ) override;
        virtual double getDouble( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Sequence< sal_Int8 > getBytes( sal_Int32 columnIndex ) override;
        virtual css::util::Date getDate( sal_Int32 columnIndex ) override;
        virtual css::util::Time getTime( sal_Int32 columnIndex ) override;
        virtual css::util::DateTime getTimestamp( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getBinaryStream( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::io::XInputStream > getCharacterStream( sal_Int32 columnIndex ) override;
        virtual cpo::uno::Any getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > getRef( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > getBlob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XClob > getClob( sal_Int32 columnIndex ) override;
        virtual css::uno::Reference< css::sdbc::XArray > getArray( sal_Int32 columnIndex ) override;
        // XCancellable
         virtual void cancel(  ) override;
        // XCloseable
        virtual void close(  ) override;
        // XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > getMetaData(  ) override;
        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;
        // XColumnLocate
        virtual sal_Int32 findColumn( const OUString& columnName ) override;
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
