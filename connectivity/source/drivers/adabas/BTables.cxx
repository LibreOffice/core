/*************************************************************************
 *
 *  $RCSfile: BTables.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:56:17 $
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

#ifndef _CONNECTIVITY_ADABAS_TABLES_HXX_
#include "adabas/BTables.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_VIEWS_HXX_
#include "adabas/BViews.hxx"
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
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
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

using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Reference< XNamed > OTables::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aSchema;
    sal_Int32 nLen = _rName.indexOf('.');
    aSchema = _rName.copy(0,nLen);
    aName   = _rName.copy(nLen+1);

    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString::createFromAscii("%");
    //  aTypes[0] = ::rtl::OUString::createFromAscii("TABLE");
    //  aTypes[1] = ::rtl::OUString::createFromAscii("SYSTEMTABLE");

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),aSchema,aName,aTypes);

    Reference< XNamed > xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one table with this name
        {
            OAdabasTable* pRet = new OAdabasTable(this, static_cast<OAdabasCatalog&>(m_rParent).getConnection(),
                                        aName,xRow->getString(4),xRow->getString(5),aSchema);
            xRet = pRet;
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OAdabasCatalog&>(m_rParent).refreshTables();
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
    return new OAdabasTable(this,static_cast<OAdabasCatalog&>(m_rParent).getConnection());
}
// -----------------------------------------------------------------------------
Reference< XNamed > OTables::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    Reference< XNamed > xName(_xDescriptor,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Must be a XName interface here !");
    return xName.is() ? createObject(xName->getName()) : Reference< XNamed >();
}
// -------------------------------------------------------------------------
// XAppend
void OTables::appendObject( const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
    if(!aName.getLength())
        ::dbtools::throwFunctionSequenceException(*this);

    createTable(descriptor);
}
// -------------------------------------------------------------------------
void OTables::setComments(const Reference< XPropertySet >& descriptor ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE TABLE ");
    ::rtl::OUString aQuote  = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );
    aSql = ::rtl::OUString::createFromAscii("COMMENT ON TABLE ");
    ::rtl::OUString sSchema;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= sSchema;
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + sDot;

    aSql += aQuote + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
            + ::rtl::OUString::createFromAscii(" '")
            + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION)))
            + ::rtl::OUString::createFromAscii("'");
    xStmt->execute(aSql);

    // columns
    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xColProp;

    aSql = ::rtl::OUString::createFromAscii("COMMENT ON COLUMN ");
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + sDot;
    aSql += aQuote + getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote  + sDot
            + aQuote;

    for(sal_Int32 i=0;i<xColumns->getCount();++i)
    {
        ::cppu::extractInterface(xColProp,xColumns->getByIndex(i));
        if(xColProp.is())
        {
            ::rtl::OUString aDescription = getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION)));
            if(aDescription.getLength())
            {
                ::rtl::OUString aCom = aSql + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                            + ::rtl::OUString::createFromAscii(" '")
                                            + aDescription
                                            + ::rtl::OUString::createFromAscii("'");
                xStmt->execute(aSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }
    }
    ::comphelper::disposeComponent(xStmt);
}
// -------------------------------------------------------------------------
// XDrop
void OTables::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(getObject(_nPos),UNO_QUERY);
    sal_Bool bIsNew = sal_False;
    if(xTunnel.is())
    {
        connectivity::sdbcx::ODescriptor* pTable = (connectivity::sdbcx::ODescriptor*)xTunnel->getSomething(connectivity::sdbcx::ODescriptor::getUnoTunnelImplementationId());
        if(pTable)
            bIsNew = pTable->isNew();
    }
    if (!bIsNew)
    {
        OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );

        ::rtl::OUString aName,aSchema;
        sal_Int32 nLen = _sElementName.indexOf('.');
        aSchema = _sElementName.copy(0,nLen);
        aName   = _sElementName.copy(nLen+1);
        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP ");
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        Reference<XPropertySet> xProp(xTunnel,UNO_QUERY);
        sal_Bool bIsView;
        if(bIsView = (xProp.is() && ::comphelper::getString(xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))) == ::rtl::OUString::createFromAscii("VIEW"))) // here we have a view
            aSql += ::rtl::OUString::createFromAscii("VIEW ");
        else
            aSql += ::rtl::OUString::createFromAscii("TABLE ");

        aSql += m_xMetaData->getIdentifierQuoteString(  ) + aSchema + m_xMetaData->getIdentifierQuoteString(  );
        aSql += sDot;
        aSql += m_xMetaData->getIdentifierQuoteString(  ) + aName + m_xMetaData->getIdentifierQuoteString(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
        // if no exception was thrown we must delete it from the views
        if(bIsView)
        {
            OViews* pViews = static_cast<OViews*>(static_cast<OAdabasCatalog&>(m_rParent).getPrivateViews());
            if(pViews && pViews->hasByName(_sElementName))
                pViews->dropByNameImpl(_sElementName);
        }
    }
}
// -------------------------------------------------------------------------
void OTables::createTable( const Reference< XPropertySet >& descriptor )
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE TABLE ");
    ::rtl::OUString aQuote  = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();
    ::rtl::OUString sSchema;

    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= sSchema;
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + sDot;
    else
        descriptor->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),makeAny(static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getUserName().
                    toAsciiUpperCase()
                    ));

    aSql += ::dbtools::quoteName(aQuote, getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))))
                + ::rtl::OUString::createFromAscii(" (");

    // columns
    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xColProp;

    Any aTypeName;
    sal_Int32 nCount = xColumns->getCount();
    if(!nCount)
        ::dbtools::throwFunctionSequenceException(*this);

    for(sal_Int32 i=0;i<nCount;++i)
    {
        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
        {

            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote;

            aSql += ::rtl::OUString::createFromAscii(" ");
            aSql += OTables::getColumnSqlType(xColProp);
            aSql += OTables::getColumnSqlNotNullDefault(xColProp);
            aSql += ::rtl::OUString::createFromAscii(",");
        }
    }

    // keys

    Reference<XKeysSupplier> xKeySup(descriptor,UNO_QUERY);

    Reference<XIndexAccess> xKeys = xKeySup->getKeys();
    if(xKeys.is())
    {
        sal_Bool bPKey = sal_False;
        for(sal_Int32 i=0;i<xKeys->getCount();++i)
        {
            if(::cppu::extractInterface(xColProp,xKeys->getByIndex(i)) && xColProp.is())
            {

                sal_Int32 nKeyType      = getINT32(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));

                if(nKeyType == KeyType::PRIMARY)
                {
                    if(bPKey)
                        throw SQLException();

                    bPKey = sal_True;
                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql += ::rtl::OUString::createFromAscii(" PRIMARY KEY (");
                    for(sal_Int32 i=0;i<xColumns->getCount();++i)
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                        +   ::rtl::OUString::createFromAscii(",");
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
                }
                else if(nKeyType == KeyType::UNIQUE)
                {
                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql += ::rtl::OUString::createFromAscii(" UNIQUE (");
                    for(sal_Int32 i=0;i<xColumns->getCount();++i)
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                        + ::rtl::OUString::createFromAscii(",");
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
                }
                else if(nKeyType == KeyType::FOREIGN)
                {
                    sal_Int32 nDeleteRule   = getINT32(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELETERULE)));

                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql += ::rtl::OUString::createFromAscii(" FOREIGN KEY ");
                    ::rtl::OUString aName,aSchema,aRefTable = getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REFERENCEDTABLE)));
                    sal_Int32 nLen = aRefTable.indexOf('.');
                    aSchema = aRefTable.copy(0,nLen);
                    aName   = aRefTable.copy(nLen+1);

                    aSql += aQuote + aSchema + aQuote + sDot
                                + aQuote + aName + aQuote
                                + ::rtl::OUString::createFromAscii(" (");

                    for(sal_Int32 i=0;i<xColumns->getCount();++i)
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                            aSql += aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                                        + ::rtl::OUString::createFromAscii(",");
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

                    switch(nDeleteRule)
                    {
                        case KeyRule::CASCADE:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE CASCADE ");
                            break;
                        case KeyRule::RESTRICT:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE RESTRICT ");
                            break;
                        case KeyRule::SET_NULL:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET NULL ");
                            break;
                        case KeyRule::SET_DEFAULT:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET DEFAULT ");
                            break;
                        default:
                            ;
                    }
                }
            }
        }
    }

    if(aSql.lastIndexOf(',') == (aSql.getLength()-1))
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
    else
        aSql += ::rtl::OUString::createFromAscii(")");

    OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    if(getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION))).getLength())
        setComments(descriptor);
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
::rtl::OUString OTables::getColumnSqlType(const Reference<XPropertySet>& _rxColProp)
{
    ::rtl::OUString sSql;
    sal_Int32 nDataType = 0;
    _rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nDataType;
    switch(nDataType)
    {
        case DataType::VARBINARY:
            sSql += ::rtl::OUString::createFromAscii("VAR");
            /* run through*/
        case DataType::BINARY:
            sSql += ::rtl::OUString::createFromAscii("CHAR");
            break;
        default:
            {
                Any aTypeName = _rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                if(aTypeName.hasValue() && getString(aTypeName).getLength())
                    sSql += getString(aTypeName);
                else
                    sSql += OTables::getTypeString(_rxColProp) + ::rtl::OUString::createFromAscii(" ");
            }
    }

    switch(nDataType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::FLOAT:
        case DataType::REAL:
            sSql += ::rtl::OUString::createFromAscii("(")
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))))
                        + ::rtl::OUString::createFromAscii(")");
            break;

        case DataType::DECIMAL:
        case DataType::NUMERIC:
            sSql += ::rtl::OUString::createFromAscii("(")
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))))
                        + ::rtl::OUString::createFromAscii(",")
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))))
                        + ::rtl::OUString::createFromAscii(")");
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
            sSql += ::rtl::OUString::createFromAscii("(")
                        + ::rtl::OUString::valueOf(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))))
                        + ::rtl::OUString::createFromAscii(") BYTE");
            break;
    }
    return sSql;
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getColumnSqlNotNullDefault(const Reference<XPropertySet>& _rxColProp)
{
    OSL_ENSURE(_rxColProp.is(),"OTables::getColumnSqlNotNullDefault: Column is null!");
    ::rtl::OUString sSql;
    ::rtl::OUString aDefault = getString(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE)));
    if(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))) == ColumnValue::NO_NULLS)
    {
        sSql += ::rtl::OUString::createFromAscii(" NOT NULL");
        if(aDefault.getLength())
            sSql += ::rtl::OUString::createFromAscii(" WITH DEFAULT");
    }
    else if(aDefault.getLength())
    {
        sSql +=::rtl::OUString::createFromAscii(" DEFAULT '") + aDefault;
        sSql += ::rtl::OUString::createFromAscii("'");
    }
    return sSql;
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getTypeString(const Reference< XPropertySet >& _rxColProp)
{
    ::rtl::OUString aValue;
    switch(getINT32(_rxColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
    {
        case DataType::BIT:
            aValue = ::rtl::OUString::createFromAscii("BOOLEAN");
            break;
        case DataType::TINYINT:
            aValue = ::rtl::OUString::createFromAscii("SMALLINT");
            break;
        case DataType::SMALLINT:
            aValue = ::rtl::OUString::createFromAscii("SMALLINT");
            break;
        case DataType::INTEGER:
            aValue = ::rtl::OUString::createFromAscii("INT");
            break;
        case DataType::FLOAT:
            aValue = ::rtl::OUString::createFromAscii("FLOAT");
            break;
        case DataType::REAL:
            aValue = ::rtl::OUString::createFromAscii("REAL");
            break;
        case DataType::DOUBLE:
            aValue = ::rtl::OUString::createFromAscii("DOUBLE");
            break;
        case DataType::NUMERIC:
            aValue = ::rtl::OUString::createFromAscii("DECIMAL");
            break;
        case DataType::DECIMAL:
            aValue = ::rtl::OUString::createFromAscii("DECIMAL");
            break;
        case DataType::CHAR:
            aValue = ::rtl::OUString::createFromAscii("CHAR");
            break;
        case DataType::VARCHAR:
            aValue = ::rtl::OUString::createFromAscii("VARCHAR");
            break;
        case DataType::LONGVARCHAR:
            aValue = ::rtl::OUString::createFromAscii("LONG VARCHAR");
            break;
        case DataType::DATE:
            aValue = ::rtl::OUString::createFromAscii("DATE");
            break;
        case DataType::TIME:
            aValue = ::rtl::OUString::createFromAscii("TIME");
            break;
        case DataType::TIMESTAMP:
            aValue = ::rtl::OUString::createFromAscii("TIMESTAMP");
            break;
        case DataType::BINARY:
            aValue = ::rtl::OUString::createFromAscii("CHAR () BYTE");
            break;
        case DataType::VARBINARY:
            aValue = ::rtl::OUString::createFromAscii("VARCHAR () BYTE");
            break;
        case DataType::LONGVARBINARY:
            aValue = ::rtl::OUString::createFromAscii("LONG BYTE");
            break;
    }
    return aValue;
}
// -----------------------------------------------------------------------------

