/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: YTables.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:53:30 $
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

#ifndef CONNECTIVITY_MYSQL_TABLES_HXX
#include "mysql/YTables.hxx"
#endif
#ifndef _CONNECTIVITY_MYSQL_VIEWS_HXX_
#include "mysql/YViews.hxx"
#endif
#ifndef CONNECTIVITY_MYSQL_TABLE_HXX
#include "mysql/YTable.hxx"
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
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef CONNECTIVITY_MYSQL_CATALOG_HXX
#include "mysql/YCatalog.hxx"
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
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace ::cppu;
using namespace connectivity::mysql;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OTables::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString sCatalog,sSchema,sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData,_rName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);

    static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
    static const ::rtl::OUString s_sTableTypeTable(RTL_CONSTASCII_USTRINGPARAM("TABLE"));
    static const ::rtl::OUString s_sAll(RTL_CONSTASCII_USTRINGPARAM("%"));

    Sequence< ::rtl::OUString > sTableTypes(3);
    sTableTypes[0] = s_sTableTypeView;
    sTableTypes[1] = s_sTableTypeTable;
    sTableTypes[2] = s_sAll;    // just to be sure to include anything else ....

    Any aCatalog;
    if ( sCatalog.getLength() )
        aCatalog <<= sCatalog;
    Reference< XResultSet > xResult = m_xMetaData->getTables(aCatalog,sSchema,sTable,sTableTypes);

    sdbcx::ObjectType xRet = NULL;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if ( xResult->next() ) // there can be only one table with this name
        {
//          Reference<XStatement> xStmt = m_xConnection->createStatement();
//          if ( xStmt.is() )
//          {
//              Reference< XResultSet > xPrivRes = xStmt->executeQuery();
//              Reference< XRow > xPrivRow(xPrivRes,UNO_QUERY);
//              while ( xPrivRes.is() && xPrivRes->next() )
//              {
//                  if ( xPrivRow->getString(1) )
//                  {
//                  }
//              }
//          }
            sal_Int32 nPrivileges = Privilege::DROP         |
                                    Privilege::REFERENCE    |
                                    Privilege::ALTER        |
                                    Privilege::CREATE       |
                                    Privilege::READ         |
                                    Privilege::DELETE       |
                                    Privilege::UPDATE       |
                                    Privilege::INSERT       |
                                    Privilege::SELECT;

            OMySQLTable* pRet = new OMySQLTable( this
                                                ,static_cast<OMySQLCatalog&>(m_rParent).getConnection()
                                                ,sTable
                                                ,xRow->getString(4)
                                                ,xRow->getString(5)
                                                ,sSchema
                                                ,sCatalog
                                                ,nPrivileges);
            xRet = pRet;
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OMySQLCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OTables::disposing(void)
{
    m_xMetaData = NULL;
    OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OTables::createEmptyObject()
{
    return new OMySQLTable(this,static_cast<OMySQLCatalog&>(m_rParent).getConnection());
}
// -------------------------------------------------------------------------
// XAppend
void OTables::appendObject( const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
    if(!aName.getLength())
        ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(this));

    createTable(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void OTables::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< XInterface > xObject( getObject( _nPos ) );
    sal_Bool bIsNew = connectivity::sdbcx::ODescriptor::isNew( xObject );
    if (!bIsNew)
    {
        Reference< XConnection > xConnection = static_cast<OMySQLCatalog&>(m_rParent).getConnection();


        ::rtl::OUString sCatalog,sSchema,sTable;
        ::dbtools::qualifiedNameComponents(m_xMetaData,_sElementName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);

        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP ");

        Reference<XPropertySet> xProp(xObject,UNO_QUERY);
        sal_Bool bIsView;
        if(bIsView = (xProp.is() && ::comphelper::getString(xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))) == ::rtl::OUString::createFromAscii("VIEW"))) // here we have a view
            aSql += ::rtl::OUString::createFromAscii("VIEW ");
        else
            aSql += ::rtl::OUString::createFromAscii("TABLE ");

        ::rtl::OUString sComposedName;
        ::dbtools::composeTableName(m_xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_True,::dbtools::eInDataManipulation);
        aSql += sComposedName;
        Reference< XStatement > xStmt = xConnection->createStatement(  );
        if ( xStmt.is() )
        {
            xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
        // if no exception was thrown we must delete it from the views
        if ( bIsView )
        {
            OViews* pViews = static_cast<OViews*>(static_cast<OMySQLCatalog&>(m_rParent).getPrivateViews());
            if ( pViews && pViews->hasByName(_sElementName) )
                pViews->dropByNameImpl(_sElementName);
        }
    }
}
// -------------------------------------------------------------------------
void OTables::createTable( const Reference< XPropertySet >& descriptor )
{
    Reference< XConnection > xConnection = static_cast<OMySQLCatalog&>(m_rParent).getConnection();
    ::rtl::OUString aSql = ::dbtools::createSqlCreateTableStatement(descriptor,xConnection);

    Reference< XStatement > xStmt = xConnection->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OTables::appendNew(const ::rtl::OUString& _rsNewTable)
{
    insertElement(_rsNewTable,NULL);

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rsNewTable), Any(), Any());
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getNameForObject(const sdbcx::ObjectType& _xObject)
{
    OSL_ENSURE(_xObject.is(),"OTables::getNameForObject: Object is NULL!");
    return ::dbtools::composeTableName(m_xMetaData,_xObject,sal_False,::dbtools::eInDataManipulation);
}
// -----------------------------------------------------------------------------

