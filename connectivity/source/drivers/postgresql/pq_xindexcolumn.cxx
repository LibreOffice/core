/* -*- Mode: C++; eval:(c-set-style "bsd"); tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
 ************************************************************************/

#include <cppuhelper/typeprovider.hxx>

#include "pq_xindexcolumn.hxx"

using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Type;
using com::sun::star::uno::Sequence;

using com::sun::star::beans::XPropertySet;

namespace pq_sdbc_driver
{
#define ASCII_STR(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )

IndexColumn::IndexColumn( const ::rtl::Reference< RefCountedMutex > & refMutex,
                      const Reference< com::sun::star::sdbc::XConnection > & connection,
                      ConnectionSettings *pSettings )
    : ReflectionBase(
        getStatics().refl.indexColumn.implName,
        getStatics().refl.indexColumn.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.indexColumn.pProps )
{}

Reference< XPropertySet > IndexColumn::createDataDescriptor(  ) throw (RuntimeException)
{
    IndexColumnDescriptor * pIndexColumn = new IndexColumnDescriptor(
        m_refMutex, m_conn, m_pSettings  );
    pIndexColumn->copyValuesFrom( this );

    return Reference< XPropertySet > ( pIndexColumn );
}


IndexColumnDescriptor::IndexColumnDescriptor(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const Reference< com::sun::star::sdbc::XConnection > & connection,
    ConnectionSettings *pSettings )
    : ReflectionBase(
        getStatics().refl.indexColumnDescriptor.implName,
        getStatics().refl.indexColumnDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.indexColumnDescriptor.pProps )
{}

Reference< XPropertySet > IndexColumnDescriptor::createDataDescriptor(  ) throw (RuntimeException)
{
    IndexColumnDescriptor * pIndexColumn = new IndexColumnDescriptor(
        m_refMutex, m_conn, m_pSettings  );
    pIndexColumn->copyValuesFrom( this );

    return Reference< XPropertySet > ( pIndexColumn );
}

}
