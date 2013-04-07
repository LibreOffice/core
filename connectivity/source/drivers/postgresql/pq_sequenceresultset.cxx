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


#include "pq_sequenceresultset.hxx"
#include "pq_sequenceresultsetmetadata.hxx"



using com::sun::star::sdbc::XResultSetMetaData;

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;

namespace pq_sdbc_driver
{

void SequenceResultSet::checkClosed()
    throw ( com::sun::star::sdbc::SQLException,
            com::sun::star::uno::RuntimeException )
{
    // we never close :o)
}


Any SequenceResultSet::getValue( sal_Int32 columnIndex )
{
    m_wasNull = ! m_data[m_row][columnIndex -1 ].hasValue();
    return m_data[m_row][columnIndex -1 ];
}

SequenceResultSet::SequenceResultSet(
    const ::rtl::Reference< RefCountedMutex > & mutex,
    const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
    const Sequence< OUString > &colNames,
    const Sequence< Sequence< Any > > &data,
    const Reference< com::sun::star::script::XTypeConverter > & tc,
    const ColumnMetaDataVector *pVec) :
    BaseResultSet( mutex, owner, data.getLength(), colNames.getLength(),tc ),
    m_data(data ),
    m_columnNames( colNames )
{
    if( pVec )
    {
        m_meta = new SequenceResultSetMetaData( mutex, *pVec, m_columnNames.getLength() );
    }
}

SequenceResultSet::~SequenceResultSet()
{

}

void SequenceResultSet::close(  )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    // a noop
}

Reference< XResultSetMetaData > SAL_CALL SequenceResultSet::getMetaData(  )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    if( ! m_meta.is() )
    {
        // Oh no, not again
        throw ::com::sun::star::sdbc::SQLException(
            "pq_sequenceresultset: no meta supported ", *this,
        // I did not find "IM001" in a specific standard,
        // but it seems to be used by other systems (such as ODBC)
        // and some parts of LibreOffice special-case it.
            OUString( "IM001" ), 1, Any() );
    }
    return m_meta;
}


sal_Int32 SAL_CALL SequenceResultSet::findColumn(
    const OUString& columnName )
    throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    // no need to guard, as all members are readonly !
    sal_Int32 ret = -1;
    for( int i = 0 ;i < m_fieldCount ; i ++ )
    {
        if( columnName == m_columnNames[i] )
        {
            ret = i+1;
            break;
        }
    }
    return ret;
}
}
