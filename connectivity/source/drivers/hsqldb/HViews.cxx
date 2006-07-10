/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HViews.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:28:56 $
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

#ifndef _CONNECTIVITY_HSQLDB_VIEWS_HXX_
#include "hsqldb/HViews.hxx"
#endif
#ifndef CONNECTIVITY_HSQLDB_TABLES_HXX
#include "hsqldb/HTables.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_CHECKOPTION_HPP_
#include <com/sun/star/sdbcx/CheckOption.hpp>
#endif
#ifndef CONNECTIVITY_HSQLDB_CATALOG_HXX
#include "hsqldb/HCatalog.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#include "connectivity/sdbcx/VView.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OViews::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData,
                                        _rName,
                                        sCatalog,
                                        sSchema,
                                        sTable,
                                        ::dbtools::eInDataManipulation);
    return new ::connectivity::sdbcx::OView(isCaseSensitive(),
                            sTable,
                            m_xMetaData,
                            0,
                            ::rtl::OUString(),
                            sSchema,
                            sCatalog
                            );
}
// -------------------------------------------------------------------------
void OViews::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OHCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OViews::disposing(void)
{
    m_xMetaData = NULL;
    OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OViews::createDescriptor()
{
    Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();
    connectivity::sdbcx::OView* pNew = new connectivity::sdbcx::OView(sal_True,xConnection->getMetaData());
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OViews::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    createView(descriptor);
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OViews::dropObject(sal_Int32 _nPos,const ::rtl::OUString /*_sElementName*/)
{
    if ( m_bInDrop )
        return;

    Reference< XInterface > xObject( getObject( _nPos ) );
    sal_Bool bIsNew = connectivity::sdbcx::ODescriptor::isNew( xObject );
    if (!bIsNew)
    {
        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP VIEW");

        Reference<XPropertySet> xProp(xObject,UNO_QUERY);
        aSql += ::dbtools::composeTableName( m_xMetaData, xProp, ::dbtools::eInTableDefinitions, false, false, true );

        Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = xConnection->createStatement(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OViews::dropByNameImpl(const ::rtl::OUString& elementName)
{
    m_bInDrop = sal_True;
    OCollection_TYPE::dropByName(elementName);
    m_bInDrop = sal_False;
}
// -----------------------------------------------------------------------------
void OViews::createView( const Reference< XPropertySet >& descriptor )
{
    Reference<XConnection> xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();

    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE VIEW ");
    ::rtl::OUString aQuote  = xConnection->getMetaData()->getIdentifierQuoteString(  );
    ::rtl::OUString sSchema,sCommand;

    aSql += ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::eInTableDefinitions, false, false, true );

    aSql += ::rtl::OUString::createFromAscii(" AS ");
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND)) >>= sCommand;
    aSql += sCommand;

    Reference< XStatement > xStmt = xConnection->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }

    // insert the new view also in the tables collection
    OTables* pTables = static_cast<OTables*>(static_cast<OHCatalog&>(m_rParent).getPrivateTables());
    if ( pTables )
    {
        ::rtl::OUString sName = ::dbtools::composeTableName( m_xMetaData, descriptor, ::dbtools::eInDataManipulation, false, false, false );
        pTables->appendNew(sName);
    }
}
// -----------------------------------------------------------------------------
void OViews::appendNew(const ::rtl::OUString& _rsNewTable)
{
    insertElement(_rsNewTable,NULL);
    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rsNewTable), Any(), Any());
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}
// -----------------------------------------------------------------------------



