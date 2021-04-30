/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 200? by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#pragma once

#include "pq_sequenceresultset.hxx"
#include "pq_resultsetmetadata.hxx"

#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>

namespace pq_sdbc_driver
{

struct UpdateableField
{
    UpdateableField( )
        : isTouched(false)
    {}
    css::uno::Any value;
    bool    isTouched;
};

typedef std::vector< UpdateableField > UpdateableFieldVector;

class UpdateableResultSet final :
        public SequenceResultSet,
        public css::sdbc::XResultSetUpdate,
        public css::sdbc::XRowUpdate
{
    ConnectionSettings **m_ppSettings;
    OUString m_schema;
    OUString m_table;
    std::vector< OUString > m_primaryKey;
    UpdateableFieldVector m_updateableField;
    bool  m_insertRow;

private:
    UpdateableResultSet(
        const ::rtl::Reference< comphelper::RefCountedMutex > & mutex,
        const css::uno::Reference< css::uno::XInterface > &owner,
        const std::vector< OUString > &colNames,
        const std::vector< std::vector< css::uno::Any > > &data,
        ConnectionSettings **ppSettings,
        const OUString &schema,
        const OUString &table,
        const std::vector< OUString > &primaryKey)
        : SequenceResultSet( mutex, owner, colNames, data, (*ppSettings)->tc ),
          m_ppSettings( ppSettings ),
          m_schema( schema ),
          m_table( table ),
          m_primaryKey( primaryKey ),
          m_insertRow( false )
    {
        // LEM TODO: this duplicates code in pq_resultset.cxx, except for different value
        //           of ResultSetConcurrency. Baaad.
        //           Why is an updatable ResultSet a sequenceresultset in the first place?
        //           This seems to imply that the whole data is fetched once and kept in memory. BAAAAD.
        // LEM TODO: shouldn't these things be inherited from the statement or something like that?
        // Positioned update/delete not supported, so no cursor name
        // Fetch direction and size are cursor-specific things, so not used now.
        // Fetch size not set
        m_props[ BASERESULTSET_FETCH_DIRECTION ] <<= css::sdbc::FetchDirection::UNKNOWN;
        // No escape processing for now
        m_props[ BASERESULTSET_ESCAPE_PROCESSING ] <<= false;
        // Bookmarks not implemented for now
        m_props[ BASERESULTSET_IS_BOOKMARKABLE ] <<= false;
        m_props[ BASERESULTSET_RESULT_SET_CONCURRENCY ] <<=
            css::sdbc::ResultSetConcurrency::UPDATABLE;
        m_props[ BASERESULTSET_RESULT_SET_TYPE ] <<=
            css::sdbc::ResultSetType::SCROLL_INSENSITIVE;
    }

    OUString buildWhereClause();
    void checkUpdate( sal_Int32 column );

public:
    static css::uno::Reference< css::sdbc::XCloseable > createFromPGResultSet(
        const ::rtl::Reference< comphelper::RefCountedMutex > & mutex,
        const css::uno::Reference< css::uno::XInterface > &owner,
        ConnectionSettings **ppSettings,
        PGresult *result,
        const OUString &schema,
        const OUString &table,
        const std::vector< OUString > &primaryKey );

public: // XInterface
    virtual void SAL_CALL acquire() noexcept override { SequenceResultSet::acquire(); }
    virtual void SAL_CALL release() noexcept override { SequenceResultSet::release(); }
    virtual css::uno::Any  SAL_CALL queryInterface(
        const css::uno::Type & reqType ) override;

public: // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8> SAL_CALL getImplementationId() override;

public: // XResultSetUpdate
    virtual void SAL_CALL insertRow(  ) override;
    virtual void SAL_CALL updateRow(  ) override;
    virtual void SAL_CALL deleteRow(  ) override;
    virtual void SAL_CALL cancelRowUpdates(  ) override;
    virtual void SAL_CALL moveToInsertRow(  ) override;
    virtual void SAL_CALL moveToCurrentRow(  ) override;

public: // XRowUpdate
    virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) override;
    virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) override;
    virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) override;
    virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) override;
    virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) override;
    virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) override;
    virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) override;
    virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) override;
    virtual void SAL_CALL updateString( sal_Int32 columnIndex, const OUString& x ) override;
    virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const css::uno::Sequence< sal_Int8 >& x ) override;
    virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const css::util::Date& x ) override;
    virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const css::util::Time& x ) override;
    virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x ) override;
    virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
    virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const css::uno::Reference< css::io::XInputStream >& x, sal_Int32 length ) override;
    virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const css::uno::Any& x ) override;
    virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const css::uno::Any& x, sal_Int32 scale ) override;

public:
    /// @throws css::uno::RuntimeException
    static css::uno::Sequence< css::uno::Type > getStaticTypes( bool updateable );

};


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
