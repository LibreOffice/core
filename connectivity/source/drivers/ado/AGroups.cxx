/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AGroups.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:13:45 $
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

#ifndef _CONNECTIVITY_ADO_GROUPS_HXX_
#include "ado/AGroups.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_GROUP_HXX_
#include "ado/AGroup.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_TABLE_HXX_
#include "ado/ATable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#include "connectivity/sdbcx/IRefreshable.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace comphelper;
using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

typedef connectivity::sdbcx::OCollection OCollection_TYPE;
// -------------------------------------------------------------------------
sdbcx::ObjectType OGroups::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoGroup(m_pCatalog,isCaseSensitive(),_rName);
}
// -------------------------------------------------------------------------
void OGroups::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OGroups::createEmptyObject()
{
    return new OAdoGroup(m_pCatalog,isCaseSensitive());
}
// -------------------------------------------------------------------------
// XAppend
void OGroups::appendObject( const Reference< XPropertySet >& descriptor )
{
    OAdoGroup* pGroup = NULL;
    if(getImplementation(pGroup,descriptor) && pGroup != NULL)
        m_aCollection.Append(pGroup->getImpl());
}
// -------------------------------------------------------------------------
// XDrop
void OGroups::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    m_aCollection.Delete(_sElementName);
}
// -----------------------------------------------------------------------------




