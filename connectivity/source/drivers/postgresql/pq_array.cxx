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

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/sdbc/XArray.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <comphelper/sequence.hxx>


#include "pq_array.hxx"
#include "pq_statics.hxx"
#include "pq_sequenceresultset.hxx"


using com::sun::star::sdbc::SQLException;
using com::sun::star::uno::Any;

using com::sun::star::uno::Sequence;
namespace pq_sdbc_driver
{


OUString Array::getBaseTypeName(  )
{
    return OUString( "varchar" );
}

sal_Int32 Array::getBaseType(  )
{
    return  css::sdbc::DataType::VARCHAR;
}

css::uno::Sequence< css::uno::Any > Array::getArray(
    const css::uno::Reference< css::container::XNameAccess >& /* typeMap */ )
{
    return comphelper::containerToSequence(m_data);
}

css::uno::Sequence< css::uno::Any > Array::getArrayAtIndex(
    sal_Int32 index,
    sal_Int32 count,
    const css::uno::Reference< css::container::XNameAccess >& /* typeMap */ )
{
    checkRange( index, count );
    return Sequence< Any > ( &m_data[index-1], count );
}

css::uno::Reference< css::sdbc::XResultSet > Array::getResultSet(
    const css::uno::Reference< css::container::XNameAccess >& typeMap )
{
    return getResultSetAtIndex( 0 , m_data.size() , typeMap );
}

css::uno::Reference< css::sdbc::XResultSet > Array::getResultSetAtIndex(
    sal_Int32 index,
    sal_Int32 count,
    const css::uno::Reference< css::container::XNameAccess >& /* typeMap */ )
{
    checkRange( index, count );
    std::vector< std::vector< Any > > ret( count );

    for( int i = 0 ; i < count ; i ++ )
    {
        std::vector< Any > row( 2 );
        row[0] <<= (sal_Int32) ( i + index );
        row[1] = m_data[i+index-1];
        ret[i] = row;
    }

    return new SequenceResultSet(
        m_refMutex, m_owner, getStatics().resultSetArrayColumnNames, ret, m_tc );
}


void Array::checkRange( sal_Int32 index, sal_Int32 count )
{
    if( index >= 1 && index -1 + count <= (sal_Int32)m_data.size() )
        return;
    throw SQLException(
        "Array::getArrayAtIndex(): allowed range for index + count "
        + OUString::number( m_data.size() )
        + ", got " + OUString::number( index )
        + " + " + OUString::number( count ),
        *this, OUString(), 1, Any());

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
