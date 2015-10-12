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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_RESULTSET_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_RESULTSET_HXX

#include "column.hxx"
#include <connectivity/warningscontainer.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>

#include <cppuhelper/propshlp.hxx>
#include <comphelper/proparrhlp.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>

namespace dbaccess
{
    typedef ::cppu::WeakComponentImplHelper< css::sdbc::XWarningsSupplier,
                                              css::sdbc::XResultSet,
                                              css::sdbc::XResultSetMetaDataSupplier,
                                              css::sdbc::XRow,
                                              css::sdbc::XCloseable,
                                              css::sdbc::XColumnLocate,
                                              css::sdbcx::XRowLocate,
                                              css::sdbcx::XColumnsSupplier,
                                              css::sdbc::XResultSetUpdate,
                                              css::sdbc::XRowUpdate,
                                              css::lang::XServiceInfo > OResultSetBase;

    //  OResultSet

    class OResultSet :  public comphelper::OBaseMutex,
                        public OResultSetBase,
                        public ::cppu::OPropertySetHelper,
                        public ::comphelper::OPropertyArrayUsageHelper < OResultSet >
    {
    protected:
        css::uno::Reference< css::uno::XInterface>            m_aStatement;

        css::uno::Reference< css::sdbc::XResultSet >          m_xDelegatorResultSet;
        css::uno::Reference< css::sdbc::XResultSetUpdate >    m_xDelegatorResultSetUpdate;
        css::uno::Reference< css::sdbc::XRow >                m_xDelegatorRow;
        css::uno::Reference< css::sdbc::XRowUpdate >          m_xDelegatorRowUpdate;

        ::dbtools::WarningsContainer    m_aWarnings;
        OColumns*                       m_pColumns;
        sal_Int32                       m_nResultSetType;
        sal_Int32                       m_nResultSetConcurrency;
        bool                        m_bIsBookmarkable : 1;

    public:
        OResultSet(const css::uno::Reference< css::sdbc::XResultSet >& _xResultSet,
                   const css::uno::Reference< css::uno::XInterface >& _xStatement,
                   bool _bCaseSensitive);
        virtual ~OResultSet();

    // css::lang::XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;

    // css::uno::XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;

    // css::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // ::cppu::OComponentHelper
        virtual void SAL_CALL disposing() override;

    // css::sdbc::XCloseable
        virtual void SAL_CALL close(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                css::uno::Any & rConvertedValue,
                                css::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const css::uno::Any& rValue )
                                    throw (css::lang::IllegalArgumentException) override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const css::uno::Any& rValue
                                                     )
                                                     throw (css::uno::Exception, std::exception) override;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // css::sdbc::XWarningsSupplier
        virtual css::uno::Any SAL_CALL getWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL clearWarnings(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XResultSetMetaDataSupplier
        virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XColumnLocate
        virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XRow
        virtual sal_Bool SAL_CALL wasNull(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getString( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual float SAL_CALL getFloat( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL getDouble( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getObject( sal_Int32 columnIndex, const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XResultSet
        virtual sal_Bool SAL_CALL next(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isBeforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isAfterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL beforeFirst(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL afterLast(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL first(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL last(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL absolute( sal_Int32 row ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL relative( sal_Int32 rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL previous(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL refreshRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL rowUpdated(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL rowInserted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL rowDeleted(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbcx::XRowLocate
        virtual css::uno::Any SAL_CALL getBookmark(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL moveToBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL moveRelativeToBookmark( const css::uno::Any& bookmark, sal_Int32 rows ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL hashBookmark( const css::uno::Any& bookmark ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XResultSetUpdate
        virtual void SAL_CALL insertRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL deleteRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL cancelRowUpdates(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL moveToInsertRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL moveToCurrentRow(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdbc::XRowUpdate
        virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateString( sal_Int32 columnIndex, const OUString& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const css::uno::Sequence< sal_Int8 >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const css::util::Date& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const css::util::Time& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const css::uno::Any& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const css::uno::Any& x, sal_Int32 scale ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    protected:
        void checkReadOnly() const;
        void checkBookmarkable() const;

    private:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_RESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
