/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AUsers.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:24:28 $
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

#ifndef _CONNECTIVITY_ADO_USERS_HXX_
#include "ado/AUsers.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_USER_HXX_
#include "ado/AUser.hxx"
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
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
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

sdbcx::ObjectType OUsers::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoUser(m_pCatalog,isCaseSensitive(),_rName);
}
// -------------------------------------------------------------------------
void OUsers::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OUsers::createDescriptor()
{
    return new OUserExtend(m_pCatalog,isCaseSensitive());
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OUsers::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OUserExtend* pUser = NULL;
    if ( !getImplementation( pUser, descriptor ) || pUser == NULL )
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not create user: invalid object descriptor." ),
            static_cast<XTypeProvider*>(this)
        );

    ADOUsers* pUsers = (ADOUsers*)m_aCollection;
    pUsers->Append(OLEVariant(pUser->getImpl()),OLEString(pUser->getPassword()));

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OUsers::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    m_aCollection.Delete(_sElementName);
}
// -------------------------------------------------------------------------


