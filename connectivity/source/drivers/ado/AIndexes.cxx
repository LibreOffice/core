/*************************************************************************
 *
 *  $RCSfile: AIndexes.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2001-05-18 08:48:07 $
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
#ifndef _CONNECTIVITY_ADO_INDEXES_HXX_
#include "ado/AIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_INDEX_HXX_
#include "ado/AIndex.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_INDEXTYPE_HPP_
#include <com/sun/star/sdbc/IndexType.hpp>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;


using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Reference< XNamed > OIndexes::createObject(const ::rtl::OUString& _rName)
{
    ADOIndex* pIndex = NULL;
    m_pCollection->get_Item(OLEVariant(_rName),&pIndex);

    Reference< XNamed > xRet = new OAdoIndex(isCaseSensitive(),m_pConnection,pIndex);

    return xRet;
}
// -------------------------------------------------------------------------
void OIndexes::impl_refresh() throw(RuntimeException)
{
    m_pCollection->Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createEmptyObject()
{
    OAdoIndex* pNew = new OAdoIndex(isCaseSensitive(),m_pConnection);
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OIndexes::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        OAdoIndex* pIndex = (OAdoIndex*)xTunnel->getSomething(OAdoIndex:: getUnoTunnelImplementationId());
        if(pIndex)
            m_pCollection->Append(OLEVariant(getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)))),
                                  OLEVariant(pIndex->getImpl()));
        else
            throw SQLException(::rtl::OUString::createFromAscii("Could not append index!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    }

    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OIndexes::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    m_pCollection->Delete(OLEVariant(elementName));

    OCollection_TYPE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OIndexes::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException(::rtl::OUString::valueOf(index),*this);

    m_pCollection->Delete(OLEVariant(index));

    OCollection_TYPE::dropByIndex(index);
}
// -------------------------------------------------------------------------


