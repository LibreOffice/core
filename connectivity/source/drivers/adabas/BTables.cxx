/*************************************************************************
 *
 *  $RCSfile: BTables.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-28 08:12:54 $
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
#define CONNECTIVITY_PROPERTY_NAME_SPACE adabas
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
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

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
    aSchema,aName,aTypes);

    Reference< XNamed > xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one table with this name
        {
            OAdabasTable* pRet = new OAdabasTable(  static_cast<OAdabasCatalog&>(m_rParent).getConnection(),
                                        aName,xRow->getString(4),xRow->getString(5),aSchema);
            xRet = pRet;
        }
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
    OAdabasTable* pNew = new OAdabasTable(static_cast<OAdabasCatalog&>(m_rParent).getConnection());
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OTables::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    ObjectMap::iterator aIter = m_aNameMap.find(aName);
    if( aIter != m_aNameMap.end())
        throw ElementExistException(aName,*this);
    if(!aName.getLength())
        ::dbtools::FunctionSequenceException(*this);

    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE TABLE ");
    ::rtl::OUString aQuote  = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
    ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii("."),sSchema;

    descriptor->getPropertyValue(PROPERTY_SCHEMANAME) >>= sSchema;
    if(sSchema.getLength())
        aSql += ::dbtools::quoteName(aQuote, sSchema) + aDot;
    else
        descriptor->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getUserName()));

    aSql += ::dbtools::quoteName(aQuote, getString(descriptor->getPropertyValue(PROPERTY_NAME)))
                + ::rtl::OUString::createFromAscii(" (");

    // columns
    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xColProp;

    Any aTypeName;
    sal_Int32 nCount = xColumns->getCount();
    if(!nCount)
        ::dbtools::FunctionSequenceException(*this);

    for(sal_Int32 i=0;i<nCount;++i)
    {
        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
        {

            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(PROPERTY_NAME)) + aQuote;

            aSql = aSql + ::rtl::OUString::createFromAscii(" ");

            aTypeName = xColProp->getPropertyValue(PROPERTY_TYPENAME);

            if(aTypeName.hasValue() && getString(aTypeName).getLength())
                aSql = aSql + getString(aTypeName);
            else
                aSql = aSql + getTypeString(xColProp) + ::rtl::OUString::createFromAscii(" ");

            switch(getINT32(xColProp->getPropertyValue(PROPERTY_TYPE)))
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                    aSql = aSql + ::rtl::OUString::createFromAscii("(")
                                + ::rtl::OUString::valueOf(getINT32(xColProp->getPropertyValue(PROPERTY_PRECISION)))
                                + ::rtl::OUString::createFromAscii(")");
                    break;

                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    aSql = aSql + ::rtl::OUString::createFromAscii("(")
                                + ::rtl::OUString::valueOf(getINT32(xColProp->getPropertyValue(PROPERTY_PRECISION)))
                                + ::rtl::OUString::createFromAscii(",")
                                + ::rtl::OUString::valueOf(getINT32(xColProp->getPropertyValue(PROPERTY_SCALE)))
                                + ::rtl::OUString::createFromAscii(")");
                    break;
            }
            ::rtl::OUString aDefault = getString(xColProp->getPropertyValue(PROPERTY_DEFAULTVALUE));
            if(getINT32(xColProp->getPropertyValue(PROPERTY_ISNULLABLE)) == ColumnValue::NO_NULLS)
            {
                aSql = aSql + ::rtl::OUString::createFromAscii(" NOT NULL");
                if(aDefault.getLength())
                    aSql = aSql + ::rtl::OUString::createFromAscii(" WITH DEFAULT");
            }
            else if(aDefault.getLength())
                aSql = aSql + ::rtl::OUString::createFromAscii(" DEFAULT ") + aDefault;

            aSql = aSql + ::rtl::OUString::createFromAscii(",");
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

                sal_Int32 nKeyType      = getINT32(xColProp->getPropertyValue(PROPERTY_TYPE));

                if(nKeyType == KeyType::PRIMARY)
                {
                    if(bPKey)
                        throw SQLException();

                    bPKey = sal_True;
                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql = aSql + ::rtl::OUString::createFromAscii(" PRIMARY KEY (");
                    for(sal_Int32 i=0;i<xColumns->getCount();++i)
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(PROPERTY_NAME)) + aQuote
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

                    aSql = aSql + ::rtl::OUString::createFromAscii(" UNIQUE (");
                    for(sal_Int32 i=0;i<xColumns->getCount();++i)
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(PROPERTY_NAME)) + aQuote
                                        + ::rtl::OUString::createFromAscii(",");
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
                }
                else if(nKeyType == KeyType::FOREIGN)
                {
                    sal_Int32 nDeleteRule   = getINT32(xColProp->getPropertyValue(PROPERTY_DELETERULE));

                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns->getCount())
                        throw SQLException();

                    aSql = aSql + ::rtl::OUString::createFromAscii(" FOREIGN KEY ");
                    ::rtl::OUString aName,aSchema,aRefTable = getString(xColProp->getPropertyValue(PROPERTY_REFERENCEDTABLE));
                    sal_Int32 nLen = aRefTable.indexOf('.');
                    aSchema = aRefTable.copy(0,nLen);
                    aName   = aRefTable.copy(nLen+1);

                    aSql = aSql + aQuote + aSchema + aQuote + aDot
                                + aQuote + aName + aQuote
                                + ::rtl::OUString::createFromAscii(" (");

                    for(sal_Int32 i=0;i<xColumns->getCount();++i)
                    {
                        if(::cppu::extractInterface(xColProp,xColumns->getByIndex(i)) && xColProp.is())
                            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(PROPERTY_NAME)) + aQuote
                                        + ::rtl::OUString::createFromAscii(",");
                    }

                    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

                    switch(nDeleteRule)
                    {
                        case KeyRule::CASCADE:
                            aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE CASCADE ");
                            break;
                        case KeyRule::RESTRICT:
                            aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE RESTRICT ");
                            break;
                        case KeyRule::SET_NULL:
                            aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE SET NULL ");
                            break;
                        case KeyRule::SET_DEFAULT:
                            aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE SET DEFAULT ");
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

    if(getString(descriptor->getPropertyValue(PROPERTY_DESCRIPTION)).getLength())
        setComments(descriptor);


    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
void OTables::setComments(const Reference< XPropertySet >& descriptor ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE TABLE ");
    ::rtl::OUString aQuote  = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
    ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

    OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );
    aSql = ::rtl::OUString::createFromAscii("COMMENT ON TABLE ")
            + aQuote + getString(descriptor->getPropertyValue(PROPERTY_SCHEMANAME)) + aQuote + aDot
            + aQuote + getString(descriptor->getPropertyValue(PROPERTY_NAME)) + aQuote
            + ::rtl::OUString::createFromAscii(" '")
            + getString(descriptor->getPropertyValue(PROPERTY_DESCRIPTION))
            + ::rtl::OUString::createFromAscii("'");
    xStmt->execute(aSql);

    // columns
    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xColProp;

    aSql = ::rtl::OUString::createFromAscii("COMMENT ON COLUMN ")
            + aQuote + getString(descriptor->getPropertyValue(PROPERTY_SCHEMANAME)) + aQuote + aDot
            + aQuote + getString(descriptor->getPropertyValue(PROPERTY_NAME)) + aQuote  + aDot
            + aQuote;

    for(sal_Int32 i=0;i<xColumns->getCount();++i)
    {
        ::cppu::extractInterface(xColProp,xColumns->getByIndex(i));
        if(xColProp.is())
        {
            ::rtl::OUString aDescription = getString(xColProp->getPropertyValue(PROPERTY_DESCRIPTION));
            if(aDescription.getLength())
            {
                ::rtl::OUString aCom = aSql + getString(xColProp->getPropertyValue(PROPERTY_NAME)) + aQuote
                                            + ::rtl::OUString::createFromAscii(" '")
                                            + aDescription
                                            + ::rtl::OUString::createFromAscii("'");
                xStmt->execute(aSql);
            }
        }
    }
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OTables::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(aIter->second.get(),UNO_QUERY);
    sal_Bool bIsNew = sal_False;
    if(xTunnel.is())
    {
        OAdabasTable* pTable = (OAdabasTable*)xTunnel->getSomething(OAdabasTable:: getUnoTunnelImplementationId());
        bIsNew = pTable->isNew();
    }
    if (!bIsNew)
    {
        OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );

        ::rtl::OUString aName,aSchema;
        sal_Int32 nLen = elementName.indexOf('.');
        aSchema = elementName.copy(0,nLen);
        aName   = elementName.copy(nLen+1);
        ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP TABLE ");
        aSql = aSql + m_xMetaData->getIdentifierQuoteString(  ) + aSchema + m_xMetaData->getIdentifierQuoteString(  );
        aSql = aSql + ::rtl::OUString::createFromAscii(".");
        aSql = aSql + m_xMetaData->getIdentifierQuoteString(  ) + aName + m_xMetaData->getIdentifierQuoteString(  );
        xStmt->execute(aSql);
    }

    OCollection_TYPE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OTables::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException();

    dropByName((*m_aElements[index]).first);
}
// -------------------------------------------------------------------------


