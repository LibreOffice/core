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

#include <cppuhelper/typeprovider.hxx>

#include "pq_xkeycolumn.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;

using com::sun::star::beans::XPropertySet;

namespace pq_sdbc_driver
{
KeyColumn::KeyColumn( const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
                      const Reference< css::sdbc::XConnection > & connection,
                      ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.keycolumn.implName,
        getStatics().refl.keycolumn.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.keycolumn.pProps )
{}

Reference< XPropertySet > KeyColumn::createDataDescriptor(  )
{
    KeyColumnDescriptor * pKeyColumn = new KeyColumnDescriptor(
        m_xMutex, m_conn, m_pSettings );
    pKeyColumn->copyValuesFrom( this );

    return Reference< XPropertySet > ( pKeyColumn );
}

KeyColumnDescriptor::KeyColumnDescriptor(
    const ::rtl::Reference< comphelper::RefCountedMutex > & refMutex,
    const Reference< css::sdbc::XConnection > & connection,
    ConnectionSettings *pSettings)
    : ReflectionBase(
        getStatics().refl.keycolumnDescriptor.implName,
        getStatics().refl.keycolumnDescriptor.serviceNames,
        refMutex,
        connection,
        pSettings,
        * getStatics().refl.keycolumnDescriptor.pProps )
{}

Reference< XPropertySet > KeyColumnDescriptor::createDataDescriptor(  )
{
    KeyColumnDescriptor * pKeyColumn = new KeyColumnDescriptor(
        m_xMutex, m_conn, m_pSettings );
    pKeyColumn->copyValuesFrom( this );

    return Reference< XPropertySet > ( pKeyColumn );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
