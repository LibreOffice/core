/*************************************************************************
 *
 *  $RCSfile: MTables.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mmaher $ $Date: 2001-10-11 10:07:54 $
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

#ifndef _CONNECTIVITY_MOZAB_TABLES_HXX_
#include "MTables.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_TABLE_HXX_
#include "MTable.hxx"
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
#ifndef _CONNECTIVITY_MOZAB_CATALOG_HXX_
#include "MCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_BCONNECTION_HXX_
#include "MConnection.hxx"
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
using namespace connectivity::mozab;
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
    // sal_Int32 nLen = _rName.indexOf('.');
    // aSchema = _rName.copy(0,nLen);
    // aName    = _rName.copy(nLen+1);
    aSchema = ::rtl::OUString::createFromAscii("%");
    aName = _rName;

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
            OTable* pRet = new OTable(  this, static_cast<OCatalog&>(m_rParent).getConnection(),
                                        aName,xRow->getString(4),xRow->getString(5),aSchema);
            xRet = pRet;
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OCatalog&>(m_rParent).refreshTables();
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
    OTable* pNew = new OTable(this, static_cast<OCatalog&>(m_rParent).getConnection());
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OTables::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
    ObjectMap::iterator aIter = m_aNameMap.find(aName);
    if( aIter != m_aNameMap.end())
        throw ElementExistException(aName,*this);
    if(!aName.getLength())
        ::dbtools::throwFunctionSequenceException(*this);

    createTable(descriptor);

    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
void OTables::setComments(const Reference< XPropertySet >& descriptor ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OTables::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OTables::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void OTables::createTable( const Reference< XPropertySet >& descriptor )
{
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

