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
 *    Copyright: 2000 by Sun Microsystems, Inc.
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

#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>

#include "pq_resultset.hxx"

namespace pq_sdbc_driver
{
/** necessary to avoid crashes in OOo, when an updateable result set is requested,
    but cannot be delivered.
 */
class FakedUpdateableResultSet :
        public ResultSet,
        public css::sdbc::XResultSetUpdate,
        public css::sdbc::XRowUpdate
{
    OUString m_aReason;

public:
    FakedUpdateableResultSet(
        const ::rtl::Reference< comphelper::RefCountedMutex > & mutex,
        const css::uno::Reference< css::uno::XInterface > &owner,
        ConnectionSettings **pSettings,
        PGresult *result,
        const OUString &schema,
        const OUString &table,
        const OUString &aReason );

public: // XInterface
    virtual void SAL_CALL acquire() noexcept override { ResultSet::acquire(); }
    virtual void SAL_CALL release() noexcept override { ResultSet::release(); }
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

};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
