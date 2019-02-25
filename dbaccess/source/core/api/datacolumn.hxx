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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_DATACOLUMN_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_DATACOLUMN_HXX

#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include "resultcolumn.hxx"
namespace dbaccess
{

    //  ODataColumn

    class ODataColumn : public OResultColumn,
                        public css::sdb::XColumn,
                        public css::sdb::XColumnUpdate
    {
        css::uno::Reference < css::sdbc::XRow >       m_xRow;
        css::uno::Reference < css::sdbc::XRowUpdate > m_xRowUpdate;
    protected:
        virtual ~ODataColumn() override;
    public:
        ODataColumn  (const css::uno::Reference < css::sdbc::XResultSetMetaData >& _xMetaData,
                      const css::uno::Reference < css::sdbc::XRow >& _xRow,
                      const css::uno::Reference < css::sdbc::XRowUpdate >& _xRowUpdate,
                      sal_Int32 _nPos,
                      const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxDBMeta);

    // css::lang::XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // css::uno::XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
        virtual void SAL_CALL acquire() throw() override { OResultColumn::acquire(); }
        virtual void SAL_CALL release() throw() override { OResultColumn::release(); }

    // css::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing() override;

    // css::sdb::XColumn
        virtual sal_Bool SAL_CALL wasNull(  ) override;
        virtual OUString SAL_CALL getString(  ) override;
        virtual sal_Bool SAL_CALL getBoolean(  ) override;
        virtual sal_Int8 SAL_CALL getByte(  ) override;
        virtual sal_Int16 SAL_CALL getShort(  ) override;
        virtual sal_Int32 SAL_CALL getInt(  ) override;
        virtual sal_Int64 SAL_CALL getLong(  ) override;
        virtual float SAL_CALL getFloat(  ) override;
        virtual double SAL_CALL getDouble(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes(  ) override;
        virtual css::util::Date SAL_CALL getDate(  ) override;
        virtual css::util::Time SAL_CALL getTime(  ) override;
        virtual css::util::DateTime SAL_CALL getTimestamp(  ) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream(  ) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream(  ) override;
        virtual css::uno::Any SAL_CALL getObject( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef(  ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob(  ) override;
        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob(  ) override;
        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray(  ) override;

    // css::sdb::XColumnUpdate
        virtual void SAL_CALL updateNull(  ) override;
        virtual void SAL_CALL updateBoolean( sal_Bool x ) override;
        virtual void SAL_CALL updateByte( sal_Int8 x ) override;
        virtual void SAL_CALL updateShort( sal_Int16 x ) override;
        virtual void SAL_CALL updateInt( sal_Int32 x ) override;
        virtual void SAL_CALL updateLong( sal_Int64 x ) override;
        virtual void SAL_CALL updateFloat( float x ) override;
        virtual void SAL_CALL updateDouble( double x ) override;
        virtual void SAL_CALL updateString( const OUString& x ) override;
        virtual void SAL_CALL updateBytes( const css::uno::Sequence< sal_Int8 >& x ) override;
        virtual void SAL_CALL updateDate( const css::util::Date& x ) override;
        virtual void SAL_CALL updateTime( const css::util::Time& x ) override;
        virtual void SAL_CALL updateTimestamp( const css::util::DateTime& x ) override;
        virtual void SAL_CALL updateBinaryStream( const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void SAL_CALL updateCharacterStream( const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void SAL_CALL updateObject( const css::uno::Any& x ) override;
        virtual void SAL_CALL updateNumericObject( const css::uno::Any& x, sal_Int32 scale ) override;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_DATACOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
