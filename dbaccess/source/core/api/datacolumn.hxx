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
#include <resultcolumn.hxx>
namespace dbaccess
{

    //  ODataColumn

    class ODataColumn : public OResultColumn,
                        public css::sdb::XColumn,
                        public css::sdb::XColumnUpdate
    {
    protected:
        css::uno::Reference < css::sdbc::XRow >       m_xRow;
        css::uno::Reference < css::sdbc::XRowUpdate > m_xRowUpdate;

        virtual ~ODataColumn();
    public:
        ODataColumn  (const css::uno::Reference < css::sdbc::XResultSetMetaData >& _xMetaData,
                      const css::uno::Reference < css::sdbc::XRow >& _xRow,
                      const css::uno::Reference < css::sdbc::XRowUpdate >& _xRowUpdate,
                      sal_Int32 _nPos,
                      const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rxDBMeta);

    // css::lang::XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;

    // css::uno::XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw() override { OResultColumn::acquire(); }
        virtual void SAL_CALL release() throw() override { OResultColumn::release(); }

    // css::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // cppu::OComponentHelper
        virtual void SAL_CALL disposing() override;

    // css::sdb::XColumn
        virtual sal_Bool SAL_CALL wasNull(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getString(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getBoolean(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int8 SAL_CALL getByte(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getShort(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getInt(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int64 SAL_CALL getLong(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual float SAL_CALL getFloat(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual double SAL_CALL getDouble(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Date SAL_CALL getDate(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::Time SAL_CALL getTime(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::util::DateTime SAL_CALL getTimestamp(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getObject( const css::uno::Reference< css::container::XNameAccess >& typeMap ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

    // css::sdb::XColumnUpdate
        virtual void SAL_CALL updateNull(  ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBoolean( sal_Bool x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateByte( sal_Int8 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateShort( sal_Int16 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateInt( sal_Int32 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateLong( sal_Int64 x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateFloat( float x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateDouble( double x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateString( const OUString& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBytes( const css::uno::Sequence< sal_Int8 >& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateDate( const css::util::Date& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateTime( const css::util::Time& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateTimestamp( const css::util::DateTime& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateBinaryStream( const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateCharacterStream( const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateObject( const css::uno::Any& x ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL updateNumericObject( const css::uno::Any& x, sal_Int32 scale ) throw(css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_DATACOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
