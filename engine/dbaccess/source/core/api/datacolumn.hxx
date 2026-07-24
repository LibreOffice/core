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
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
        virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override;

    // css::uno::XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override { OResultColumn::acquire(); }
        virtual void release() noexcept override { OResultColumn::release(); }

    // css::lang::XServiceInfo
        virtual OUString getImplementationName(  ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    // cppu::OComponentHelper
        virtual void disposing() override;

    // css::sdb::XColumn
        virtual bool wasNull(  ) override;
        virtual OUString getString(  ) override;
        virtual bool getBoolean(  ) override;
        virtual sal_Int8 getByte(  ) override;
        virtual sal_Int16 getShort(  ) override;
        virtual sal_Int32 getInt(  ) override;
        virtual sal_Int64 getLong(  ) override;
        virtual float getFloat(  ) override;
        virtual double getDouble(  ) override;
        virtual cpo::uno::Sequence< sal_Int8 > getBytes(  ) override;
        virtual css::util::Date getDate(  ) override;
        virtual css::util::Time getTime(  ) override;
        virtual css::util::DateTime getTimestamp(  ) override;
        virtual css::uno::Reference< css::io::XInputStream > getBinaryStream(  ) override;
        virtual css::uno::Reference< css::io::XInputStream > getCharacterStream(  ) override;
        virtual cpo::uno::Any getObject( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        virtual css::uno::Reference< css::sdbc::XRef > getRef(  ) override;
        virtual css::uno::Reference< css::sdbc::XBlob > getBlob(  ) override;
        virtual css::uno::Reference< css::sdbc::XClob > getClob(  ) override;
        virtual css::uno::Reference< css::sdbc::XArray > getArray(  ) override;

    // css::sdb::XColumnUpdate
        virtual void updateNull(  ) override;
        virtual void updateBoolean( bool x ) override;
        virtual void updateByte( sal_Int8 x ) override;
        virtual void updateShort( sal_Int16 x ) override;
        virtual void updateInt( sal_Int32 x ) override;
        virtual void updateLong( sal_Int64 x ) override;
        virtual void updateFloat( float x ) override;
        virtual void updateDouble( double x ) override;
        virtual void updateString( const OUString& x ) override;
        virtual void updateBytes( const cpo::uno::Sequence< sal_Int8 >& x ) override;
        virtual void updateDate( const css::util::Date& x ) override;
        virtual void updateTime( const css::util::Time& x ) override;
        virtual void updateTimestamp( const css::util::DateTime& x ) override;
        virtual void updateBinaryStream( const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void updateCharacterStream( const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
        virtual void updateObject( const cpo::uno::Any& x ) override;
        virtual void updateNumericObject( const cpo::uno::Any& x, sal_Int32 scale ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
