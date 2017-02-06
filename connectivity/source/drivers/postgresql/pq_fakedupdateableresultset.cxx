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

#include "pq_fakedupdateableresultset.hxx"
#include <com/sun/star/sdbc/SQLException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

using osl::MutexGuard;


using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XResultSetUpdate;
using com::sun::star::sdbc::XRowUpdate;

namespace pq_sdbc_driver
{

FakedUpdateableResultSet::FakedUpdateableResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const css::uno::Reference< css::uno::XInterface > &owner,
        ConnectionSettings **pSettings,
        PGresult *result,
        const OUString &schema,
        const OUString &table,
        const OUString &aReason )
    : ResultSet( mutex, owner, pSettings, result, schema, table ),
      m_aReason( aReason )
{}


css::uno::Any  FakedUpdateableResultSet::queryInterface(
    const css::uno::Type & reqType )
{
    Any ret = ResultSet::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< XResultSetUpdate * > ( this ),
            static_cast< XRowUpdate * > ( this ) );
    return ret;
}


css::uno::Sequence< css::uno::Type > FakedUpdateableResultSet::getTypes()
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<XResultSetUpdate>::get(),
                cppu::UnoType<XRowUpdate>::get(),
                ResultSet::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();

}

css::uno::Sequence< sal_Int8> FakedUpdateableResultSet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void FakedUpdateableResultSet::insertRow(  )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateRow(  )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::deleteRow(  )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
 }

void FakedUpdateableResultSet::cancelRowUpdates(  )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::moveToInsertRow(  )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::moveToCurrentRow(  )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}


void FakedUpdateableResultSet::updateNull( sal_Int32 /* columnIndex */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateBoolean( sal_Int32 /* columnIndex */, sal_Bool /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateByte( sal_Int32 /* columnIndex */, sal_Int8 /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateShort( sal_Int32 /* columnIndex */, sal_Int16 /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateInt( sal_Int32 /* columnIndex */, sal_Int32 /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateLong( sal_Int32 /* columnIndex */, sal_Int64 /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateFloat( sal_Int32 /* columnIndex */, float /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateDouble( sal_Int32 /* columnIndex */, double /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateString( sal_Int32 /* columnIndex */, const OUString& /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateBytes( sal_Int32 /* columnIndex */, const css::uno::Sequence< sal_Int8 >& /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateDate( sal_Int32 /* columnIndex */, const css::util::Date& /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateTime( sal_Int32 /* columnIndex */, const css::util::Time& /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateTimestamp( sal_Int32 /* columnIndex */, const css::util::DateTime& /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateBinaryStream( sal_Int32 /* columnIndex */, const css::uno::Reference< css::io::XInputStream >& /* x */, sal_Int32 /* length */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateCharacterStream( sal_Int32 /* columnIndex */, const css::uno::Reference< css::io::XInputStream >& /* x */, sal_Int32 /* length */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateObject( sal_Int32 /* columnIndex */, const css::uno::Any& /* x */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateNumericObject( sal_Int32 /* columnIndex */, const css::uno::Any& /* x */, sal_Int32 /* scale */ )
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
