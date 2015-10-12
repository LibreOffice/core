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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_UPDATEABLERESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_UPDATEABLERESULTSET_HXX

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
    com::sun::star::uno::Any value;
    bool    isTouched;
};

typedef ::std::vector< UpdateableField > UpdateableFieldVector;

class UpdateableResultSet :
        public SequenceResultSet,
        public com::sun::star::sdbc::XResultSetUpdate,
        public com::sun::star::sdbc::XRowUpdate
{
    ConnectionSettings **m_ppSettings;
    OUString m_schema;
    OUString m_table;
    com::sun::star::uno::Sequence< OUString > m_primaryKey;
    UpdateableFieldVector m_updateableField;
    bool  m_insertRow;

protected:
    UpdateableResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
        const com::sun::star::uno::Sequence< OUString > &colNames,
        const com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::uno::Any > > &data,
        ConnectionSettings **ppSettings,
        const OUString &schema,
        const OUString &table,
        const com::sun::star::uno::Sequence< OUString > &primaryKey)
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
        sal_Bool b = sal_False;
        // Positioned update/delete not supported, so no cursor name
        // Fetch direction and size are cursor-specific things, so not used now.
        // Fetch size not set
        m_props[ BASERESULTSET_FETCH_DIRECTION ] = com::sun::star::uno::makeAny(
            com::sun::star::sdbc::FetchDirection::UNKNOWN);
        // No escape processing for now
        m_props[ BASERESULTSET_ESCAPE_PROCESSING ] = com::sun::star::uno::Any( &b, cppu::UnoType<bool>::get() );
        // Bookmarks not implemented for now
        m_props[ BASERESULTSET_IS_BOOKMARKABLE ] = com::sun::star::uno::Any( &b, cppu::UnoType<bool>::get() );
        m_props[ BASERESULTSET_RESULT_SET_CONCURRENCY ] = com::sun::star::uno::makeAny(
            com::sun::star::sdbc::ResultSetConcurrency::UPDATABLE );
        m_props[ BASERESULTSET_RESULT_SET_TYPE ] = com::sun::star::uno::makeAny(
            com::sun::star::sdbc::ResultSetType::SCROLL_INSENSITIVE );
    }

    OUString buildWhereClause();
    void checkUpdate( sal_Int32 column );

public:
    static com::sun::star::uno::Reference< com::sun::star::sdbc::XCloseable > createFromPGResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
        ConnectionSettings **ppSettings,
        PGresult *result,
        const OUString &schema,
        const OUString &table,
        const com::sun::star::uno::Sequence< OUString > &primaryKey );

public: // XInterface
    virtual void SAL_CALL acquire() throw() override { SequenceResultSet::acquire(); }
    virtual void SAL_CALL release() throw() override { SequenceResultSet::release(); }
    virtual com::sun::star::uno::Any  SAL_CALL queryInterface(
        const com::sun::star::uno::Type & reqType )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

public: // XTypeProvider
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

public: // XResultSetUpdate
    virtual void SAL_CALL insertRow(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateRow(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL deleteRow(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancelRowUpdates(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL moveToInsertRow(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL moveToCurrentRow(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XRowUpdate
    virtual void SAL_CALL updateNull( sal_Int32 columnIndex ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateFloat( sal_Int32 columnIndex, float x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateDouble( sal_Int32 columnIndex, double x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateString( sal_Int32 columnIndex, const OUString& x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateBytes( sal_Int32 columnIndex, const ::com::sun::star::uno::Sequence< sal_Int8 >& x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateBinaryStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateCharacterStream( sal_Int32 columnIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateNumericObject( sal_Int32 columnIndex, const ::com::sun::star::uno::Any& x, sal_Int32 scale ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public:

    static com::sun::star::uno::Sequence< com::sun::star::uno::Type > getStaticTypes( bool updateable )
        throw( com::sun::star::uno::RuntimeException );

};



}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_UPDATEABLERESULTSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
