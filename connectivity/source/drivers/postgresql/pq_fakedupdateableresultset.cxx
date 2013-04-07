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
 *    Version: MPL 1.1 / GPLv3+ / LGPLv2.1+
 *
 *    The contents of this file are subject to the Mozilla Public License Version
 *    1.1 (the "License"); you may not use this file except in compliance with
 *    the License or as specified alternatively below. You may obtain a copy of
 *    the License at http://www.mozilla.org/MPL/
 *
 *    Software distributed under the License is distributed on an "AS IS" basis,
 *    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *    for the specific language governing rights and limitations under the
 *    License.
 *
 *    Major Contributor(s):
 *    [ Copyright (C) 2011 Lionel Elie Mamane <lionel@mamane.lu> ]
 *
 *    All Rights Reserved.
 *
 *    For minor contributions see the git repository.
 *
 *    Alternatively, the contents of this file may be used under the terms of
 *    either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 *    the GNU Lesser General Public License Version 2.1 or later (the "LGPLv2.1+"),
 *    in which case the provisions of the GPLv3+ or the LGPLv2.1+ are applicable
 *    instead of those above.
 *
 ************************************************************************/

#include "pq_fakedupdateableresultset.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

using osl::MutexGuard;


using com::sun::star::uno::Reference;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;

using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XResultSetUpdate;
using com::sun::star::sdbc::XRowUpdate;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XStatement;

namespace pq_sdbc_driver
{

FakedUpdateableResultSet::FakedUpdateableResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
        ConnectionSettings **pSettings,
        PGresult *result,
        const OUString &schema,
        const OUString &table,
        const OUString &aReason )
    : ResultSet( mutex, owner, pSettings, result, schema, table ),
      m_aReason( aReason )
{}


com::sun::star::uno::Any  FakedUpdateableResultSet::queryInterface(
    const com::sun::star::uno::Type & reqType )
    throw (com::sun::star::uno::RuntimeException)
{
    Any ret = ResultSet::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface(
            reqType,
            static_cast< XResultSetUpdate * > ( this ),
            static_cast< XRowUpdate * > ( this ) );
    return ret;
}


com::sun::star::uno::Sequence< com::sun::star::uno::Type > FakedUpdateableResultSet::getTypes()
        throw( com::sun::star::uno::RuntimeException )
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( (Reference< XResultSetUpdate> *) 0 ),
                getCppuType( (Reference< XRowUpdate> *) 0 ),
                ResultSet::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();

}

com::sun::star::uno::Sequence< sal_Int8> FakedUpdateableResultSet::getImplementationId()
        throw( com::sun::star::uno::RuntimeException )
{
    static cppu::OImplementationId *pId;
    if( ! pId )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( ! pId )
        {
            static cppu::OImplementationId id(sal_False);
            pId = &id;
        }
    }
    return pId->getImplementationId();
}

void FakedUpdateableResultSet::insertRow(  ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateRow(  ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::deleteRow(  ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
 }

void FakedUpdateableResultSet::cancelRowUpdates(  ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::moveToInsertRow(  ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::moveToCurrentRow(  ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}


void FakedUpdateableResultSet::updateNull( sal_Int32 /* columnIndex */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateBoolean( sal_Int32 /* columnIndex */, sal_Bool /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateByte( sal_Int32 /* columnIndex */, sal_Int8 /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateShort( sal_Int32 /* columnIndex */, sal_Int16 /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateInt( sal_Int32 /* columnIndex */, sal_Int32 /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateLong( sal_Int32 /* columnIndex */, sal_Int64 /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateFloat( sal_Int32 /* columnIndex */, float /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateDouble( sal_Int32 /* columnIndex */, double /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateString( sal_Int32 /* columnIndex */, const OUString& /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateBytes( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Sequence< sal_Int8 >& /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateDate( sal_Int32 /* columnIndex */, const ::com::sun::star::util::Date& /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateTime( sal_Int32 /* columnIndex */, const ::com::sun::star::util::Time& /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateTimestamp( sal_Int32 /* columnIndex */, const ::com::sun::star::util::DateTime& /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateBinaryStream( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& /* x */, sal_Int32 /* length */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateCharacterStream( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& /* x */, sal_Int32 /* length */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateObject( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Any& /* x */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

void FakedUpdateableResultSet::updateNumericObject( sal_Int32 /* columnIndex */, const ::com::sun::star::uno::Any& /* x */, sal_Int32 /* scale */ ) throw (SQLException, RuntimeException)
{
    throw SQLException( m_aReason, *this, OUString(),1,Any() );
}

}
