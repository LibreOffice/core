/*************************************************************************
 *
 *  $RCSfile: AColumns.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 14:07:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#include "ado/AColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif

using namespace connectivity::ado;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Reference< XNamed > OColumns::createObject(const ::rtl::OUString& _rName)
{
    ADOColumn* pColumn = NULL;
    m_pCollection->get_Item(OLEVariant(_rName),&pColumn);

    Reference< XNamed > xRet = new OAdoColumn(isCaseSensitive(),pColumn);

    return xRet;
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(RuntimeException)
{
    m_pCollection->Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createEmptyObject()
{
    OAdoColumn* pNew = new OAdoColumn(isCaseSensitive());
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OColumns::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        OAdoColumn* pColumn = (OAdoColumn*)xTunnel->getSomething(OAdoColumn::getUnoTunnelImplementationId());
        m_pCollection->Append(OLEVariant(pColumn->getColumnImpl()));
    }

    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OColumns::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    m_pCollection->Delete(OLEVariant(elementName));

    OCollection_TYPE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OColumns::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException(::rtl::OUString::valueOf(index),*this);

    m_pCollection->Delete(OLEVariant(index));
    OCollection_TYPE::dropByIndex(index);
}
// -----------------------------------------------------------------------------


