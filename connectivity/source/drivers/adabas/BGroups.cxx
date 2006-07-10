/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BGroups.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:21:41 $
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

#ifndef _CONNECTIVITY_ADABAS_GROUPS_HXX_
#include "adabas/BGroups.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_GROUP_HXX_
#include "adabas/BGroup.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CONNECTIVITY_SDBCX_IREFRESHABLE_HXX_
#include "sdbcx/IRefreshable.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;
// -------------------------------------------------------------------------
sdbcx::ObjectType OGroups::createObject(const ::rtl::OUString& _rName)
{
    return new OAdabasGroup(m_pConnection,_rName);
}
// -------------------------------------------------------------------------
void OGroups::impl_refresh() throw(RuntimeException)
{
    m_pParent->refreshGroups();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OGroups::createDescriptor()
{
    //  OAdabasGroup* pNew =
    return new OAdabasGroup(m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OGroups::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& /*descriptor*/ )
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE USERGROUP ");
    ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );

    aSql = aSql + aQuote + _rForName + aQuote;

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OGroups::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("DROP USERGROUP ");
    ::rtl::OUString aQuote  = m_pConnection->getMetaData()->getIdentifierQuoteString(  );

    aSql = aSql + aQuote + _sElementName + aQuote;

    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);
}
// -------------------------------------------------------------------------


