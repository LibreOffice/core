/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "mysql/YTables.hxx"
#include "mysql/YViews.hxx"
#include "mysql/YTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include "mysql/YCatalog.hxx"
#include <comphelper/extract.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/types.hxx>
#include "TConnection.hxx"

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
m_xMetaData.clear();
    OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OTables::createDescriptor()
{
    return new OMySQLTable(this,static_cast<OMySQLCatalog&>(m_rParent).getConnection());
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OTables::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    createTable(descriptor);
    return createObject( _rForName );
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

        ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "DROP " ));

        Reference<XPropertySet> xProp(xObject,UNO_QUERY);
        sal_Bool bIsView = xProp.is() && ::comphelper::getString(xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))) == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
        if(bIsView) // here we have a view
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW "));
        else
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE "));

        ::rtl::OUString sComposedName(
            ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_True, ::dbtools::eInDataManipulation ) );
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
::rtl::OUString OTables::adjustSQL(const ::rtl::OUString& _sSql)
{
    ::rtl::OUString sSQL = _sSql;
    static const ::rtl::OUString s_sUNSIGNED(RTL_CONSTASCII_USTRINGPARAM("UNSIGNED"));
    sal_Int32 nIndex = sSQL.indexOf(s_sUNSIGNED);
    while(nIndex != -1 )
    {
        sal_Int32 nParen = sSQL.indexOf(')',nIndex);
        sal_Int32 nPos = nIndex + s_sUNSIGNED.getLength();
        ::rtl::OUString sNewUnsigned( sSQL.copy(nPos,nParen - nPos + 1));
        sSQL = sSQL.replaceAt(nIndex,s_sUNSIGNED.getLength()+sNewUnsigned.getLength(),sNewUnsigned + s_sUNSIGNED);
        nIndex = sSQL.indexOf(s_sUNSIGNED,nIndex + s_sUNSIGNED.getLength()+sNewUnsigned.getLength());
    }
    return sSQL;
}
// -------------------------------------------------------------------------
void OTables::createTable( const Reference< XPropertySet >& descriptor )
{
    const Reference< XConnection > xConnection = static_cast<OMySQLCatalog&>(m_rParent).getConnection();
    static const ::rtl::OUString s_sCreatePattern(RTL_CONSTASCII_USTRINGPARAM("(M,D)"));
    const ::rtl::OUString aSql = adjustSQL(::dbtools::createSqlCreateTableStatement(descriptor,xConnection,this,s_sCreatePattern));
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
    return ::dbtools::composeTableName( m_xMetaData, _xObject, ::dbtools::eInDataManipulation, false, false, false );
}
// -----------------------------------------------------------------------------
void OTables::addComment(const Reference< XPropertySet >& descriptor,::rtl::OUStringBuffer& _rOut)
{
    ::rtl::OUString sDesc;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION))     >>= sDesc;
    if ( sDesc.getLength() )
    {
        _rOut.appendAscii(" COMMENT '");
        _rOut.append(sDesc);
        _rOut.appendAscii("'");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
