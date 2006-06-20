/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AIndexes.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:14:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

sdbcx::ObjectType OIndexes::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoIndex(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}
// -------------------------------------------------------------------------
void OIndexes::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createEmptyObject()
{
    return new OAdoIndex(isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
void OIndexes::appendObject( const Reference< XPropertySet >& descriptor )
{
    OAdoIndex* pIndex = NULL;
    sal_Bool bError = sal_True;
    if(getImplementation(pIndex,descriptor) && pIndex != NULL)
    {
        ADOIndexes* pIndexes = m_aCollection;
        bError = FAILED(pIndexes->Append(OLEVariant(getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)))),
                                  OLEVariant(pIndex->getImpl())));
    }
    if(bError)
        throw SQLException(::rtl::OUString::createFromAscii("Could not append index!"),static_cast<XTypeProvider*>(this),OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
}
// -------------------------------------------------------------------------
// XDrop
void OIndexes::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    m_aCollection.Delete(_sElementName);
}
// -------------------------------------------------------------------------
sdbcx::ObjectType OIndexes::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    OAdoIndex* pIndex = NULL;
    if(getImplementation(pIndex,_xDescriptor) && pIndex != NULL)
        return new OAdoIndex(isCaseSensitive(),m_pConnection,pIndex->getImpl());
    return sdbcx::ObjectType();
}
// -----------------------------------------------------------------------------


