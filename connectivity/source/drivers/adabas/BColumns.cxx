/*************************************************************************
 *
 *  $RCSfile: BColumns.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-01 06:20:31 $
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

#ifndef _CONNECTIVITY_ADABAS_COLUMNS_HXX_
#include "adabas/BColumns.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLES_HXX_
#include "adabas/BTables.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif

using namespace ::comphelper;


using namespace connectivity::adabas;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Reference< XNamed > OColumns::createObject(const ::rtl::OUString& _rName)
{
    Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getColumns(Any(),
    m_pTable->getSchema(),m_pTable->getTableName(),_rName);

    Reference< XNamed > xRet = NULL;
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                sal_Int32 nType             = xRow->getInt(5);
                ::rtl::OUString sTypeName   = xRow->getString(6);
                static const ::rtl::OUString sDecimal = ::rtl::OUString::createFromAscii("DECIMAL");
                if(nType == DataType::DECIMAL && sTypeName == sDecimal)
                    nType = DataType::NUMERIC;

                OColumn* pRet = new OColumn(_rName,
                                            sTypeName,
                                            xRow->getString(13),
                                            xRow->getInt(11),
                                            xRow->getInt(7),
                                            xRow->getInt(9),
                                            nType,
                                            sal_False,sal_False,sal_False,sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createEmptyObject()
{
    return new OColumn(sal_True);
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OColumns::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(m_pTable,"OColumns::appendByDescriptor: Table is null!");
    OSL_ENSURE(descriptor.is(),"OColumns::appendByDescriptor: descriptor is null!");

    if(descriptor.is() && !m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString sQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        m_pTable->beginTransAction();
        try
        {
            ::rtl::OUString sColumnName;
            descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sColumnName;
            aSql += ::dbtools::quoteName(sQuote,m_pTable->getSchema()) + sDot + ::dbtools::quoteName(sQuote,m_pTable->getTableName());
            aSql += ::rtl::OUString::createFromAscii(" ADD (");
            aSql += ::dbtools::quoteName(sQuote,sColumnName);
            aSql += ::rtl::OUString::createFromAscii(" ");
            aSql += OTables::getColumnSqlType(descriptor);
            aSql += ::rtl::OUString::createFromAscii(" )");

            Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement();
            xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);

            m_pTable->alterNotNullValue(getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))),sColumnName);
        }
        catch(const Exception&)
        {
            m_pTable->rollbackTransAction();
            throw;
        }
        m_pTable->endTransAction();
    }
    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OColumns::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(m_pTable,"OColumns::dropByName: Table is null!");
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString sQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        aSql += ::dbtools::quoteName(sQuote,m_pTable->getSchema()) + sDot + ::dbtools::quoteName(sQuote,m_pTable->getTableName());
        aSql += ::rtl::OUString::createFromAscii(" DROP ");
        aSql += ::dbtools::quoteName(sQuote,elementName);

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
    }

    OCollection_TYPE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OColumns::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException(::rtl::OUString::valueOf(index),*this);

    OSL_ENSURE(m_pTable,"OColumns::dropByIndex: Table is null!");
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString sQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        aSql += ::dbtools::quoteName(sQuote,m_pTable->getSchema()) + sDot + ::dbtools::quoteName(sQuote,m_pTable->getTableName());
        aSql += ::rtl::OUString::createFromAscii(" DROP ");
        aSql += ::dbtools::quoteName(sQuote,m_aElements[index]->first);

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
    }
    OCollection_TYPE::dropByIndex(index);
}
// -----------------------------------------------------------------------------



