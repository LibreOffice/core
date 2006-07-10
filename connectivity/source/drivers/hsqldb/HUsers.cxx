/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HUsers.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:28:43 $
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

#ifndef _CONNECTIVITY_HSQLDB_USERS_HXX_
#include "hsqldb/HUsers.hxx"
#endif
#ifndef _CONNECTIVITY_HSQLDB_USER_HXX_
#include "hsqldb/HUser.hxx"
#endif
#ifndef CONNECTIVITY_HSQLDB_TABLE_HXX
#include "hsqldb/HTable.hxx"
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
#include "connectivity/dbexception.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

OUsers::OUsers( ::cppu::OWeakObject& _rParent,
                ::osl::Mutex& _rMutex,
                const TStringVector &_rVector,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                connectivity::sdbcx::IRefreshableUsers* _pParent)
    : sdbcx::OCollection(_rParent,sal_True,_rMutex,_rVector)
    ,m_xConnection(_xConnection)
    ,m_pParent(_pParent)
{
}
// -----------------------------------------------------------------------------

sdbcx::ObjectType OUsers::createObject(const ::rtl::OUString& _rName)
{
    return new OHSQLUser(m_xConnection,_rName);
}
// -------------------------------------------------------------------------
void OUsers::impl_refresh() throw(RuntimeException)
{
    m_pParent->refreshUsers();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OUsers::createDescriptor()
{
    OUserExtend* pNew = new OUserExtend(m_xConnection);
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OUsers::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("GRANT USAGE ON * TO ");
    ::rtl::OUString aQuote  = m_xConnection->getMetaData()->getIdentifierQuoteString(  );
    ::rtl::OUString sUserName( _rForName );
    aSql += ::dbtools::quoteName(aQuote,sUserName)
                + ::rtl::OUString::createFromAscii(" @\"%\" ");
    ::rtl::OUString sPassword;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PASSWORD)) >>= sPassword;
    if ( sPassword.getLength() )
    {
        aSql += ::rtl::OUString::createFromAscii(" IDENTIFIED BY '");
        aSql += sPassword;
        aSql += ::rtl::OUString::createFromAscii("'");
    }

    Reference< XStatement > xStmt = m_xConnection->createStatement(  );
    if(xStmt.is())
        xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OUsers::dropObject(sal_Int32 /*nPos*/,const ::rtl::OUString _sElementName)
{
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("REVOKE ALL ON * FROM ");
        ::rtl::OUString aQuote  = m_xConnection->getMetaData()->getIdentifierQuoteString(  );
        aSql += ::dbtools::quoteName(aQuote,_sElementName);

        Reference< XStatement > xStmt = m_xConnection->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

// -------------------------------------------------------------------------
